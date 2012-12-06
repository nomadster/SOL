/**  \file 
 *   \brief Implementazione del server
 *
 *    \author Alessandro Lensi - 297110
*/

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "./common.h"
#include "./dgraph.h"
#include "./shortestpath.h"
#include "./hashutenti.h"
#include "./comsock.h"

/** Il grafo delle strade */
static graph_t* strade;			

/** Flag per la terminazione del server */
static volatile sig_atomic_t quit=0;


/** Handler per SIGINT e SIGTERM */
static void sig_handler(int sig){
	write(1,"Inizio terminazione programma...\n",33);
	quit=1;
}


/** -= PER LO SCAMBIO DEI MESSAGGI TRA WORKERS E MATCH =- */
pthread_mutex_t mutexLista = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t listaNonVuota = PTHREAD_COND_INITIALIZER;

/** Un elemento della lista di scambio */
typedef struct ee {
	/** Il messaggio ovvero una richiesta o una offerta secondo il formato in specifica */
	char* mm;
	/** L'utente che effettua la richiesta o l'offerta */
	char* usr;
	/** Puntatore al prossimo elemento nella lista */
	struct ee* next;
} elem_t;

/** La lista di scambio dei messaggi */
typedef struct lista {
	/** Lista che contiene tutte le offerte ricevute */
	elem_t* offerte;
	/** Lista che contiene tutte le richieste ricevute */
	elem_t* richieste;
} lista_t;

/** La lista dei messaggi */
static volatile lista_t listaMessaggi;



/** Inizializza la lista dei messaggi (richieste/offerte) */
static void init_lista(){
	listaMessaggi.offerte=NULL;
	listaMessaggi.richieste=NULL;
}

/** Crea un nuovo elemento da aggiungere in una delle due liste 
 *  \param mm Il messaggio secondo uno dei formati in specifica
 *  \param usr L'utente che ha effettuato la richiesta o l'offerta
 *
 *  \retval ret L'elemento pronto per essere inserito in una lista
 *  \retval NULL Se ci sono stati errori
 * 		 errno == ENOMEM Malloc ha fallito
 */
static elem_t* new_elem(char* mm,char* usr){
	elem_t* ret=NULL;
	if( (ret=(elem_t*)malloc(sizeof(elem_t)))==NULL)
		return NULL;
	if( (ret->mm=(char*)malloc( strlen(mm)+1 )) == NULL ){
		free(ret);
		return NULL;
	}
	if( (ret->usr=(char*)malloc( strlen(usr)+1)) == NULL){
		free(ret->mm);
		free(ret);
		return NULL;
	}
	strcpy(ret->usr,usr);
	strcpy(ret->mm,mm);
	ret->next=NULL;
	return ret;
}


/** Elimina l'elemento puntato, ponendolo a NULL
 *  \param ee Puntatore al puntatore all'elemento da eliminare
 */
static void free_elem(elem_t ** ee){
	if( (*ee) == NULL)
		return;
	free( (*ee)->mm);
	free( (*ee)->usr);
	free( (*ee) );
	(*ee)=NULL;
	return;
}


/** Inserisce un nuovo elemento nella lista delle offerte, allocando la struttura dati necessaria.
 *  \param mm Il messaggio nel formato "SORGENTE:DESTINAZIONE:posti"
 *  \param usr L'utente che effettua l'offerta
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore
 *	       errno == EINVAL Parametri errati
 *             errno == ENOMEM Malloc fallita
 */
static int inserisci_offerta(char* mm,char* usr){
	elem_t* n=NULL;
	EC_R( mm==NULL || usr == NULL,EINVAL,DONOTHING,-1 );
	if( (n=new_elem(mm,usr)) == NULL){
		STAMPA(stderr,"Offerta %s di %s:\nImpossibile creare le risorse necessarie.\n",mm,usr);
		errno=ENOMEM;
		return -1;
	}
	n->next=listaMessaggi.offerte;
	listaMessaggi.offerte=n;
	return 0;
}

/** Inserisce un nuovo elemento nella lista delle richieste, allocando la struttura dati necessaria.
 *  \param mm Il messaggio nel formato "SORGENTE:DESTINAZIONE"
 *  \param usr L'utente che effettua la richiesta
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore
 *	       errno == EINVAL Parametri errati
 *             errno == ENOMEM Malloc fallita
 */
static int inserisci_richiesta(char* mm,char* usr){
	elem_t* n=NULL;
	EC_R( mm==NULL || usr==NULL,EINVAL,DONOTHING,-1 );
	if( (n=new_elem(mm,usr)) == NULL ){
		STAMPA(stderr,"Richiesta %s di %s:\nImpossibile creare le risorse necessarie.\n",mm,usr);
		errno=ENOMEM;
		return -1;
	}
	n->next=listaMessaggi.richieste;
	listaMessaggi.richieste=n;
	return 0;
}

/** Preleva una offerta dalla lista delle offerte 
 *
 *  \retval o L'offerta prelevata
 *  \retval NULL Se non ci sono più offerte da prelevare
 */
static elem_t* preleva_offerta(){
	elem_t* r=NULL;
	if( listaMessaggi.offerte == NULL )
		return NULL;
	r=listaMessaggi.offerte;
	listaMessaggi.offerte=listaMessaggi.offerte->next;
	r->next=NULL;
	return r;
}

/** Preleva una richiesta dalla lista delle richieste
 *
 *  \retval o La richiesta prelevata
 *  \retval NULL Se non ci sono più richieste da prelevare
 */
static elem_t* preleva_richiesta(){
	elem_t* r=NULL;
	if( listaMessaggi.richieste == NULL )
		return NULL;
	r=listaMessaggi.richieste;
	listaMessaggi.richieste=listaMessaggi.richieste->next;
	r->next=NULL;
	return r;
}
	


/** -= GESTIONE_UTENTI =- */

/** Messaggi comunemente usati già allocati e pronti all'utilizzo */
/** Richiesta accettata */
static message_t msgok;
/** Richiesta non accettata */
static message_t msgno;
/** Password errata */
static message_t msgbadpwd;
/** Utente già connesso */
static message_t msgconn;
/** Matching terminato */
static message_t msgsharend;

/** Inizializza i messaggi globali */
static void init_global_msg(){
	msgok.type=MSG_OK;
	msgok.length=0;
	msgok.buffer=NULL;
	
	msgno.type=MSG_NO;
	msgno.length=0;
	msgno.buffer=NULL;
	
	msgbadpwd.type=MSG_BADPWD;
	msgbadpwd.buffer=NULL;
	msgbadpwd.length=0;
	
	msgconn.type=MSG_CONNECTED;
	msgconn.buffer=NULL;
	msgconn.length=0;


	msgsharend.type=MSG_SHAREND;
	msgsharend.length=0;
	msgsharend.buffer=NULL;
}



/** Per accedere in mutua esclusione alla hashtable degli utenti*/
pthread_mutex_t mutexUtenti = PTHREAD_MUTEX_INITIALIZER;
/** La hashtable degli utenti */
hashtable_t hashtable;

/** Esegue il login dell'utente.
 * \param usr Nome utente
 * \param pwd Password
 * \param skt Nome del socket
 *
 * \retval 0 Se il login è avvenuto con successo
 * \post hashtable contiene una entry per la tripla <usr,pwd,skt>
 * \retval -1 In caso di errore
 *            errno == EALREADY Se l'utente è già connesso
 *            errno == EBADE Password errata
 *            errno == ENOMEM Malloc fallita
 */
static int login(char* usr,char* pwd,char* skt){
	user_t* cu=NULL;
	if( (cu=user_exist(usr,&hashtable)) == NULL ) {
 		EC_R( (cu=create_user(usr,pwd,skt)) == NULL,DONTSET,DONOTHING,-1 );
 		
		EC_R( add_user(cu,&hashtable) == -1,DONTSET,{ free_user(&cu); },-1 );
		return 0;
	}
	else {
		EC_R( cu->is_connected == 1 ,EALREADY,DONOTHING,-1 );
		EC_R( check_password(cu,pwd) == 0,EBADE,DONOTHING,-1 );
		EC_R( update_user(cu,skt) == -1,DONTSET,DONOTHING,-1 );
	 }
	return 0;
}

static void logout(char* usr){
	user_t* cu=NULL;
	if( (cu=user_exist(usr,&hashtable)) == NULL )
		return;
	cu->is_connected=0;
}
/** Segnala a tutti gli utenti connessi che il match è stato eseguito, cioè invia a tutti MSG_SHAREND */
static void segnala_match(){

	int i;
	int skt;
	user_t* usr=NULL;
	pthread_mutex_lock(&mutexUtenti);
	for(i=0;i<N_HEAD;i++){
		usr=hashtable.head[i];
		while(usr!=NULL){
			if( usr->socket!=NULL){
				if( (skt=openConnection(usr->socket)) != -1 ){
					sendMessage(skt,&msgsharend);
					closeSocket(skt);
				}
			}
			usr=usr->next;
		}
	}
	pthread_mutex_unlock(&mutexUtenti);
}
/*******************************/

/** -= GESTIONE DEI WORKER =- */

/** Segnala che si può creare un nuovo worker */
pthread_cond_t workerLiberi = PTHREAD_COND_INITIALIZER;
/** Segnala che non ci sono più worker attivi */
pthread_cond_t ultimoWorker = PTHREAD_COND_INITIALIZER;
/** Per la mutua esclusione sulle variabili di condizione */
pthread_mutex_t mutexWorker = PTHREAD_MUTEX_INITIALIZER;
/** Numero di worker attivi */
static volatile sig_atomic_t activeWorker=0;



/** Incrementa il numero di worker attivi */
static void start_worker(){
	/*pthread_mutex_lock(&mutexWorker);*/
	activeWorker++;
	/*fprintf(stderr,"\a\t\t\t------------start--------------->%d\n",activeWorker);*/
	/*pthread_mutex_unlock(&mutexWorker);*/
}

/** Decrementa il numero di worker attivi e segnala sulla variabile di condizione giusta
 *  lo stato di activeWorker */
static void quit_worker(){
/*	pthread_mutex_lock(&mutexWorker); */
	activeWorker--;
	/*fprintf(stderr,"\a\t\t\t------------stop---------------->%d\n",activeWorker);*/
	pthread_mutex_lock(&mutexWorker);
	pthread_cond_signal(&workerLiberi);
	if( activeWorker == 0)
		pthread_cond_signal(&ultimoWorker);
	
	pthread_mutex_unlock(&mutexWorker);
}	

/** Attende la terminazione di tutti i worker (activeWorker==0) */
static void aspetta_ultimo_worker(){
	/*struct timespec ts;*/
	pthread_mutex_lock(&mutexWorker);
	while( activeWorker > 0)
		pthread_cond_wait(&ultimoWorker,&mutexWorker);
		/*pthread_cond_timedwait(&ultimoWorker,&mutexWorker,&ts);*/
	pthread_mutex_unlock(&mutexWorker);
}

/** Attende la ricezione di MSG_CONNECT 
 *  \param skt Il socket sul quale il client è connesso
 *  \param msg Puntatore al message_t sul quale salvare il messaggio 
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 Se errore (setta errno)
 */
static int attendi_connect(int skt, message_t* msg){
	
	if( receiveMessage(skt,msg) == -1)
		return -1;
	
	if( msg->type!=MSG_CONNECT){
		sendMessage(skt,&msgno);
		return -1;
	}
	return 0;
}		
	

	
/** Fa terminare il worker stampando il messaggio mm */
static void worker_die(char* usr,int skt,char* mm){
	
	STAMPA(stderr,"Worker(%s): %s\n",usr,((mm==NULL)?"GoodBye!":mm));
	quit_worker();
	closeSocket(skt);
}

	
/** Thread worker. */
void* worker(void* arg){
	
	char my_usr[LUSERNAME+1]; 	/** L'utente gestito da questo worker */
	char my_sck[LSOCKET+1];		/** Il nome della socket del client */
	char my_pwd[LPASS+1];		/** La password dell'utente */
	int skt;			/** Il socket sul quale siamo connessi */
	message_t rmsg;			/** Per salvarci il messaggio che riceviamo */
	sigset_t t;			/** Per mascherare i segnali */
	int error=-1;			/** Per il valore di ritorno delle funzioni */
	int user_logout=0;		/** Segnala la terminazione di questo worker per volontà del client
	                                 *  o per un errore interno al worker
	                                 */
	if( arg == NULL ){
		STAMPA(stderr,"ARGOMENTO THREAD WORKER è NULL!!!!!\n");
		return (void*)-1;
	}

	skt=(*(int*)(arg));
	free(arg);
	rmsg.buffer=NULL;

	start_worker();
	
	/** Maschero i segnali */
	sigemptyset(&t);
	sigaddset(&t,SIGINT);
	sigaddset(&t,SIGTERM);
	sigaddset(&t,SIGPIPE);
	sigaddset(&t,SIGUSR1);
	pthread_sigmask(SIG_SETMASK,&t,NULL);
	

	if( attendi_connect(skt,&rmsg) == -1 ){		
		worker_die("???",skt,"connessione in ingresso fallita\n");
		return (void*)-1;
	}
	/** Copio i dati arrivati con MSG_CONNECT*/
	strcpy(my_usr,rmsg.buffer);
	strcpy(my_pwd,&(rmsg.buffer[strlen(my_usr)+1]));
	strcpy(my_sck,&(rmsg.buffer[strlen(my_usr)+1+strlen(my_pwd)+1]));
	free_msg_buffer(&rmsg);
	STAMPA(stdout,"Worker(%s): ricevuto MSG_CONNECT pwd:%s, skt:%s\n",my_usr,my_pwd,my_sck);
	/* Controllo i dati di accesso forniti e rispondo di conseguenza*/
	pthread_mutex_lock(&mutexUtenti);
	error=login(my_usr,my_pwd,my_sck);
 	pthread_mutex_unlock(&mutexUtenti);
 	 
	if( error == 0 ){
 	 	if( sendMessage(skt,&msgok) == -1 ){
			worker_die(my_usr,skt,"Errore di comunicazione\n");
			quit_worker();
			return (void*)-1;
		}
	}
	else {
		if( errno == ENOMEM ){
			sendMessage(skt,&msgno);
			worker_die(my_usr,skt,"Mancano le risorse necessarie per creare l'utente");
		}
		else if( errno == EBADE ){
			sendMessage(skt,&msgbadpwd);
			worker_die(my_usr,skt,"Inserita password sbagliata");
		}
		else if( errno == EALREADY) {
			sendMessage(skt,&msgconn);
			worker_die(my_usr,skt,"Utente già connesso");
		}
		quit_worker();
		return (void*)-1;
	}
	
	/** Adesso rimango in attesa di messaggi da parte del client */
	while( user_logout == 0 && receiveMessage(skt,&rmsg) != -1 && quit == 0){
	
		switch(rmsg.type){
		
			case MSG_EXIT:
				STAMPA(stdout,"Worker(%s): client ha richiesto logout\n",my_usr);
				free_msg_buffer(&rmsg);
				user_logout=1;
				break;

			case MSG_REQUEST:
				pthread_mutex_lock(&mutexLista);
				STAMPA(stdout,"Worker(%s): ricevuta richiesta %s\n",my_usr,rmsg.buffer);
				error=inserisci_richiesta(rmsg.buffer,my_usr);
				free_msg_buffer(&rmsg);
				if( error == -1 ){
					if( sendMessage(skt,&msgno) == -1 ){
						pthread_mutex_unlock(&mutexLista);
						STAMPA(stderr,"Worker(%s): errore di comunicazione\
						                 con il client\n",my_usr);
						user_logout=1;
					}
				}
				else {
					if( sendMessage(skt,&msgok) == -1 ){
						pthread_mutex_unlock(&mutexLista);
						STAMPA(stderr,"Worker(%s): errore di comunicazione\
						                 con il client\n",my_usr);
						user_logout=1;
					}
					else 
						pthread_cond_signal(&listaNonVuota);
				}
				pthread_mutex_unlock(&mutexLista);
				break;
				
			case MSG_OFFER:
				STAMPA(stdout,"Worker(%s): ricevuta offerta %s\n",my_usr,rmsg.buffer);
				
				pthread_mutex_lock(&mutexLista);
				error=inserisci_offerta(rmsg.buffer,my_usr);
				pthread_mutex_unlock(&mutexLista);
				
				free_msg_buffer(&rmsg);

				if( error == -1 ){
					if( sendMessage(skt,&msgno) == -1 ){
					        pthread_mutex_unlock(&mutexLista);
						STAMPA(stderr,"Worker(%s): errore di comunicazione\
						                 con il client\n",my_usr);
						user_logout=1;
					}
				}
				else {
					if( sendMessage(skt,&msgok) == -1 ){
						pthread_mutex_unlock(&mutexLista);
						STAMPA(stderr,"Worker(%s): errore di comunicazione\
						                 con il client\n",my_usr);
						user_logout=1;
					}
					else
						pthread_cond_signal(&listaNonVuota);
				}
				break;

			default:
				free_msg_buffer(&rmsg);
				STAMPA(stderr,"Worker(%s): ricevuto un messaggio senza significato\n",my_usr);
				if( sendMessage(skt,&msgno) == -1 ){
					STAMPA(stderr,"Worker(%s): errore di comunicazione\
						                 con il client\n",my_usr);
					user_logout=1;
					
				}
				break;
		}
	}
	STAMPA(stdout,"Worker(%s): Chiudo!\n",my_usr);
	pthread_mutex_lock(&mutexUtenti);
	logout(my_usr);
	pthread_mutex_unlock(&mutexUtenti);
	quit_worker();
	closeSocket(skt);
	return (void*)0;
}
/*******************************/


/** -= GESTIONE DEL MATCH =- */
/** Segnala la terminazione del thread match */
static volatile sig_atomic_t quit_match=0;


static void elabora_offerte(){
	
	elem_t* elem=NULL;
	char* source=NULL;
	char* dest=NULL;
	char* posti=NULL;
	char* saveptr=NULL;
	pthread_mutex_lock(&mutexLista);
	/** prelevo prima tutte le offerte e poi le richieste. Così aumento la probabilità
	 *  che una richiesta venga soddisfatta */
	while( (elem=preleva_offerta()) != NULL ){
		pthread_mutex_unlock(&mutexLista);
		saveptr=NULL;
		/** Estraggo source,dest,posti dal messaggio dell'offerta */
		source=strtok_r(elem->mm,":",&saveptr);
		dest=strtok_r(NULL,":",&saveptr);
		posti=strtok_r(NULL,":",&saveptr);
		if( aggiungi_share_path(strade,source,dest,elem->usr,atoi(posti)) == -1 ){
			if( errno == EBADR ){
				STAMPA(stderr,"Match(): Non esiste il cammino da %s a %s\n\
				                Offerta non inserita\n",source,dest);
			}
			else { 
				STAMPA(stderr,"Match(): Errore interno, offerta %s:%s:%s non inserita\n",source,dest,posti);
			}
		}
		else{
			STAMPA(stdout,"Match(): offerta %s:%s:%s inserita con successo\n",source,dest,posti);
		}
		free_elem(&elem);
		pthread_mutex_lock(&mutexLista);
	}
	pthread_mutex_unlock(&mutexLista);
}


static void elabora_richieste(FILE* log){

	elem_t* elem=NULL;
	char* source=NULL;
	char* dest=NULL;
	char* gentile_utente=NULL;
	char* saveptr=NULL;
	char** passaggi=NULL;
	
	user_t* offerente=NULL;
	user_t* richiedente=NULL;

	int i;
	int cskt;/** Per aprire la connessione con il client c*/	
	message_t outmsg; /** Per inviare l'accoppiamento */
	
	outmsg.type=MSG_SHARE;
	outmsg.length=0;
	outmsg.buffer=NULL;
	
	pthread_mutex_lock(&mutexLista);
	while( (elem=preleva_richiesta()) != NULL ){
		pthread_mutex_unlock(&mutexLista);
		saveptr=NULL;
		/** Estraggo source e dest dal messaggio della richiesta */
		source=strtok_r(elem->mm,":",&saveptr);
		dest=strtok_r(NULL,":",&saveptr);
		if( (passaggi=trova_un_passaggio(strade,source,dest,elem->usr)) == NULL ){
			if( errno != 0){
				STAMPA(stderr,"Match(): richiesta %s NON VALIDA\n",elem->mm);
			}
			else{
				STAMPA(stderr,"Match(): Impossibile soddisfare la richiesta %s:%s\n",source,dest);
			}
		}
		else {
			/** Devo notificare il match SIA al richiedente CHE a TUTTI gli offerenti.
			 *  Per farlo ho bisogno del socket di tutti.
			 */
			pthread_mutex_lock(&mutexUtenti);
			if( (richiedente=user_exist(elem->usr,&hashtable)) == NULL){
				STAMPA(stderr,"Match(): il richiedente %s si è disconnesso!\n",elem->usr);
			}
			pthread_mutex_unlock(&mutexUtenti);
			i=0;
			/** Adesso devo notificare al richiedente e a tutti gli offerenti l'accoppiamento */
			while(passaggi[i] != NULL){
			
				outmsg.buffer=malloc(strlen(passaggi[i])+1);
				strcpy(outmsg.buffer,passaggi[i]);
				outmsg.length=(strlen(outmsg.buffer)+1);
				
				/** Salvo nel log l'accoppiamento */
				fprintf(log,"%s\n",outmsg.buffer);
				fflush(log);					
				
				if( richiedente != NULL ){			
					if( (cskt=openConnection(richiedente->socket)) != -1 ){
						sendMessage(cskt,&outmsg);
						closeSocket(cskt);
					}
				}
				saveptr=NULL;
				gentile_utente=strtok_r(passaggi[i],"$",&saveptr);
				pthread_mutex_lock(&mutexUtenti);
				if( (offerente=user_exist(gentile_utente,&hashtable)) == NULL ){
					STAMPA(stderr,"Match(): l'offerente %s si\
					                è disconnesso!\n",gentile_utente);
				}
				pthread_mutex_unlock(&mutexUtenti);				
								
				if( offerente != NULL){
					if( (cskt=openConnection(offerente->socket)) != -1){
						sendMessage(cskt,&outmsg);
						closeSocket(cskt);
					}
				}
					
				free(passaggi[i]);
				free_msg_buffer(&outmsg);
				i++;
			} /* fine while passaggi[i] */
			free(passaggi[i]);
			free(passaggi);
		} /* fine else */
			
		free_elem(&elem);
	
		pthread_mutex_lock(&mutexLista);
	} /* fine while preleva_richiesta */
	pthread_mutex_unlock(&mutexLista);
}

void* match(void* arg){
	
	struct timespec ts;		/** Lo uso nella sigtimedwait */
	FILE* log=NULL;			/** Punterà al file di log, già aperto, passato come parametro */
	sigset_t s;			/** Per mascherare i segnali */

	if( arg == NULL ){
		STAMPA(stderr,"Argomento del match è NULL!\n");
		return (void*)-1;
	}
	log=(FILE*)arg;
	
	ts.tv_sec=SEC_TIMER;
	ts.tv_nsec=0;

	
	/** Maschero i segnali */
	sigemptyset(&s);
	sigaddset(&s,SIGINT);
	sigaddset(&s,SIGTERM);
	sigaddset(&s,SIGUSR1);
	sigaddset(&s,SIGPIPE);
	pthread_sigmask(SIG_SETMASK,&s,NULL);
	
	/** Il match si sblocca dopo SEC_TIMER o dopo che ha ricevuto SIGUSR1.*/
	sigemptyset(&s);
	sigaddset(&s,SIGUSR1);

	while( quit_match == 0){
	
		sigtimedwait(&s,NULL,&ts);
		STAMPA(stdout,"MATCH(): Inizio routine accoppiamenti\n");
		
		elabora_offerte();
		elabora_richieste(log);
		
		STAMPA(stdout,"MATCH(): Fine routine accoppiamenti\n");
		segnala_match();
	}
	STAMPA(stdout,"Match(): Esco\n");
	return (void*)0;
}
			
			


int main(int argc, char* argv[]){

	FILE* fd1=NULL,*fd2=NULL;		/** I file per caricare il grafo */
	FILE* log=NULL;				/** Per il file di log del match */
	int mgskt;				/** Socket sul quale si accettano le connessioni dei client */
	int connSkt;				/** Socket su cui troviamo un client connesso */
	sigset_t set;				/** Per mascherare SIGPIPE */
	struct sigaction signals;		/** Per installare l'handler */
	pthread_t worker_pid;			/** Per eseguire il detach dei worker */		
	pthread_t pid_match;			/** Per terminare il match */
	
	int* worker_arg;			/** Per passare gli argomenti ai worker */

	EC_R(  argc != 3 ,DONTSET,
		{ STAMPA(stderr,"Parametri errati!\nInvocare con %s filecittà filestrade\n",argv[0]);},-1);
	EC_R( (fd1=fopen(argv[1],"r")) == NULL,DONTSET,
		{ STAMPA(stderr,"Errore nell'apertura del file strade");},-1);
		
	EC_R( (fd2=fopen(argv[2],"r")) == NULL ,DONTSET,
			{ STAMPA(stderr,"Errore nell'apertura del file strade");},-1 );
		

	/* Inizializzo e strutture dati per il server */	
	EC_R( (strade=load_graph(fd1,fd2))==NULL,DONTSET,{ perror("Creazione grafo"); },-1 );
	fclose(fd1);
	fclose(fd2);

	init_lista();
	EC_R( init_hashtable(&hashtable)==-1,DONTSET,
	      { STAMPA(stderr,"Errore inizializzazione hashtable utenti"); },-1 );

	init_global_msg();

	EC_R( (log=fopen(DEFAULT_LOG_FILE,"w")) == NULL,DONTSET,
		{ STAMPA(stdout,"Impossibile aprire file di log!!!!\n");}, -1);
		
	
	/* Maschero i segnali */
	bzero(&signals,sizeof(signals));
	signals.sa_handler=sig_handler;
	EC_R( sigaction(SIGINT,&signals,NULL) == -1 || sigaction(SIGTERM,&signals,NULL) == -1,DONTSET,
		{ STAMPA(stderr,"Fallita l'installazione dell'handler\n"); }, -1);


	sigemptyset(&set);
	sigaddset(&set,SIGPIPE);
	sigaddset(&set,SIGUSR1);
	pthread_sigmask(SIG_SETMASK,&set,NULL);
	

	/* Creo il server socket e avvio il thread worker */

	EC_R( (mgskt=createServerChannel(DEFAULT_SOCKET)) == -1,DONTSET,{ perror("Creazione Socket"); }, -1 );
		
	pthread_create(&pid_match,NULL,match,(void*)log);
	
	while( quit == 0 ){
		
		if( activeWorker < MAX_ACTIVE_WORKER ){
 
 			/** Con SIGPIPE mascherato, i problemi di comunicazione
 			    li riscontro se acceptConnection torna -1 */
			if( (connSkt=acceptConnection(mgskt)) == -1 ){
				break;
			}
			if( (worker_arg=(int*)malloc(sizeof(int))) == NULL ){
				closeSocket(connSkt);
				continue;
			}
			else {
				(*worker_arg)=connSkt;
				pthread_create(&worker_pid,NULL,worker,(void*)(worker_arg));
				pthread_detach(worker_pid);
			}
		} 
		else {
			pthread_mutex_lock(&mutexWorker);
			pthread_cond_wait(&workerLiberi,&mutexWorker);
			pthread_mutex_unlock(&mutexWorker);
		}
	}
	closeSocket(mgskt);

	aspetta_ultimo_worker();
	STAMPA(stdout,"Threads worker terminati\n");
	quit_match=1;
	pthread_kill(pid_match,SIGUSR1);
	pthread_join(pid_match,NULL);
	STAMPA(stdout,"Thread match terminato\n");
	
	free_graph( &strade );
	free_hashtable(&hashtable);
	unlink(DEFAULT_SOCKET);
	STAMPA(stdout,"Bye!\n");

	fclose(log);
	return 0;
	
}	
	
	
	


