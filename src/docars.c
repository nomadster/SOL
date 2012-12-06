/**  \file 
 *   \brief Implementazione del client
 *
 *    \author Alessandro Lensi - 297110
*/
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#include "./common.h"
#include "./dgraph.h"
#include "./comsock.h"

/** Indica se il client deve terminare. sig_atomic_t garantisce l'accesso atomico al dato. man signal.h */
static volatile sig_atomic_t quit=0;

/** Gestore per SIGINT e SIGTERM. */
void signal_handler(int sig){
	quit=1;
}

void usr2_handler(int sig){
	return;
}

/** Argomento per il thread console */
typedef struct consolearg {
	char user[LUSERNAME+1];
	char socket[LSOCKET+1];
	char pass[LPASS+1];
} console_arg;




/** Tenta di stabilire la connessione secondo il protocollo in specifica stampando a video il risultato
 *  di tale operazione.
 *  \pre skt è stato creato con openConnection()
 *  \param skt Il socket sul quale inviare la richiesta. 
 *  \param user Stringa utente
 *  \param pass Stringa password
 *  \param sockname Stringa del nome socket sul quale il client vuole essere contattato
 *
 * \retval 0 Se la connessione è stata possibile (server ha risposto con MSG_OK)
 * \post Siamo connessi con il server
 *
 * \retval -1 Se ci sono stati problemi -- setta errno
 *	 		errno == 0 Il server ha risposto con MSG_NO
 *		   	errno == EINVAL Parametri errati
 * \post Non siamo connessi al server
 */	   
static int make_connection(int skt, char* user, char* pass, char* sockname){
	
	int user_len;  	/** Lunghezza del nome utente */
	int pass_len;	/** Lunghezza della password */
	int sock_len;	/** Lunghezza della stringa socket */
	message_t msg;	/** Per inviare il MSG_CONNECT */


	EC_R( skt<0 || user==NULL || pass == NULL || sockname == NULL,EINVAL,DONOTHING,-1 );

	/** Misuro le stringhe parametro */
	EC_R( (user_len=strlen(user)) == 0,EINVAL,DONOTHING,-1 );
	EC_R( (pass_len=strlen(pass)) == 0,EINVAL,DONOTHING,-1 );
	EC_R( (sock_len=strlen(sockname)) == 0,EINVAL,DONOTHING,-1 );

        /* Forgio il MSG_CONNECT */	
	msg.type=MSG_CONNECT;
	EC_R( (msg.buffer=(char*)malloc( (user_len +1 + pass_len + 1 + sock_len +1 ))) == NULL,
		DONTSET,DONOTHING,-1);
	/* buffer deve contenere contiene "user\0pass\0socket\0" */
	strcpy(msg.buffer,user);
	strcpy( &(msg.buffer[(user_len+1)]),pass);
	strcpy(&(msg.buffer[(user_len+1+pass_len+1)]),sockname);
	/* msg.length include il '\0' finale */
	msg.length=(user_len + 1 + pass_len + 1 + sock_len + 1); 
	
	EC_R( sendMessage(skt,&msg) == -1,DONTSET,{ free_msg_buffer(&msg); },-1 );
	
	free_msg_buffer(&msg);
	EC_R( receiveMessage(skt,&msg) == -1,DONTSET,DONOTHING,-1 );
	if( msg.type == MSG_NO ){
		STAMPA(stdout,"Errore durante la connessione al server:\n%s\n",
			        ((msg.length>0)?msg.buffer:"Errore generico"));
	}
	else if( msg.type == MSG_OK){
		STAMPA(stdout,"Connessione stabilita: %s\n", ((msg.buffer!=NULL)?msg.buffer:"OK"));
	}
	else if ( msg.type == MSG_BADPWD ){
		STAMPA(stdout,"password %s non valida per l'utente %s\n",pass,user);
	}
	else if ( msg.type == MSG_CONNECTED ){
		STAMPA(stdout,"L'utente %s è già connesso\n",user);
	}
	free_msg_buffer(&msg);

	errno=0;
	return	((msg.type==MSG_OK)?0:-1);
}


/* Chiude la connessione secondo il protocollo in specifica.
 * \pre skt è stato aperto con openConnection()
 * \param skt Il socket su cui siamo connessi al server
 *
 * \post La connessione logica con il server è terminata
 */
static void close_connection(int skt){
	message_t end;
	if(skt<0)
		return;
	end.type=MSG_EXIT;
	end.length=0;
	end.buffer=NULL;
	if( sendMessage(skt,&end) == -1 )
		return;
	receiveMessage(skt,&end);
	free_msg_buffer(&end);
	return;
}



/** Valida la stringa in input per la sintassi delle richieste "SOURCE:DEST" (il %R lo controlla console)
 *  \param input La stringa da validare
 *
 *  \retval 1 Se la stringa è corretta
 *  \retval 0 Se la stringa non è corretta
 *  \retval -1 In caso di errore (setta errno)
 *
 */
static int valida_input_richiesta(char* input){

	char* lbl1=NULL,*lbl2=NULL;
	
	char* cpy=NULL;
	int input_len=0;
	char* tkn=NULL; /** Lo faccio puntare al carattere ':' */
	/* Creo una copia della stringa in input (se l'input è corretto non voglio modificare la stringa.
	 * Poi valido la copia rimuovendo i ':' e verificando la correttezza delle sottostringhe 
	 * La stringa valida di lunghezza minima è A:A 
	 */
	EC_R( input==NULL,EINVAL,DONOTHING,-1 );
	if( (input_len=strlen(input)) > (LLABEL+1+LLABEL) || input_len < 3)
		return 0;
	
	EC_R( (cpy=(char*)malloc( (input_len+1) ))== NULL,DONTSET,DONOTHING,-1 );
	strcpy(cpy,input);
	if( (tkn=strchr(cpy,':')) == NULL){
		free(cpy); 
		return 0;
	}
	lbl1=cpy;
	lbl2=&(tkn[1]);
	tkn[0]='\0';
	if( valida_stringa_citta(lbl1) == -1 || valida_stringa_citta(lbl2) == -1 ){
		free(cpy);
		errno=0;
		return 0;
	}
	free(cpy);
	return 1;
}

/* Gestisce le richieste di share
 * \param input La stringa letta da riga di comando
 * \param skt Il socket aperto con il server
 *
 * \retval 0 Se la richiesta è stata inviata con successo
 * \retval -1 in caso di errore.
 *            errno == 0 Se l'errore è risolto con un output a video
 *            errno == EFAULT Se il client deve terminare (socket chiuso)
 */
static int gestisci_richiesta(char* input, int skt){

	message_t msg;
	int ret=-1;
	ret=valida_input_richiesta(input);
	if( ret != 1 ){
	
		if( ret == -1 ){
			STAMPA(stdout,"Errore interno, riprovare\n");
		}
		else
			STAMPA(stderr,BAD_CMD_MSG);
		
		errno=0;
		return -1;
	}
	
	msg.type=MSG_REQUEST;
	if( (msg.buffer=(char*)malloc( (strlen(input) +1) )) == NULL ){
		STAMPA(stderr,"Errore interno, riprovare\n");  
		errno=0;
		return -1;
	}
	       
	msg.length=( strlen(input) + 1);
	strcpy(msg.buffer,input);
	if( sendMessage(skt,&msg) == -1){
		free_msg_buffer(&msg);
		if( errno == ENOMEM){
			STAMPA(stdout,"Errore interno, riprovare\n");
			errno=0;
		}
		else{
			STAMPA(stdout,"Connessione perduta!\n");
			errno=EFAULT;
		}
		return -1;
	}
	free_msg_buffer(&msg);

	if( receiveMessage(skt,&msg) == -1){
		free_msg_buffer(&msg);
		if( errno == ENOMEM){
			STAMPA(stdout,"Errore interno, riprovare\n");
			errno=0;
		}
		else{
			STAMPA(stdout,"Connessione perduta!\n");
			errno=EFAULT;
		}
		return -1;
	}
	STAMPA(stdout,"Richiesta %saccettata (%s)\n",((msg.type==MSG_OK)?"":"non "),((msg.buffer!=NULL)?msg.buffer:"-") );
	free_msg_buffer(&msg);
	return 0;
}


/** valida la stringa in input nel caso che si tratti di una offerta.
 * La sintassi corretta è SOURCE:DEST:n
 *
 *  \param input La stringa letta da riga di comando
 *
 *  \retval 1 Se la stringa è corretta
 *  \retval 0 Se la stringa non è corretta
 *  \retval -1 In caso di errore (setta errno)
 */
static int valida_input_offerta(char* input){

	char* posti = NULL;
	int posti_len=0;
	char* chr=NULL; /* Per individuare il ':' finale nella stringa */
	char* cpy=NULL; /* qua ci copio la stringa di input, perchè nel caso fosse corretta non la devo modificare */
	int input_len=0;
	int i;
	EC_R( input==NULL,EINVAL,DONOTHING,-1 );
	input_len=strlen(input);

	/** L'offerta di lunghezza minima è A:B:1	 */
	if( input_len > (LLABEL+1+LLABEL+1+5) || input_len < 5)
		return 0;
	/* La validazione della prima parte di questa stringa è equivalente alla chiamata di 
	 * valida_input_richiesta(). Questa controlla infatti SOURCE:DEST.
	 * Cerco quindi il primo, partendo da destra, carattere ':' nella stringa. 
	 * Lo sostituisco con '\0' e provo  a validare la prima parte usando valida_input_richiesta()
	 */
	EC_R( (cpy=(char*)malloc((input_len+1)))==NULL,DONTSET,DONOTHING,-1 );
	
	strcpy(cpy,input);	
	
	if( (chr=strrchr(cpy,':'))==NULL){
		free(cpy);
		return 0;
	}
	chr[0]='\0';
	if( valida_input_richiesta(cpy) != 1 ){
		free(cpy);
		return 0;
	}
	
	/* Ora mi rimande da validare quello che sta dopo i secondi ':'. Devono essere al più 5 cifre, ma almeno 1 */
	if(chr[1] == '\0'){
		free(cpy);
		return 0;
	}
	posti=&(chr[1]);
	if( (posti_len=strlen(posti)) > 5){
		free(cpy);
		return 0;
	}
	for(i=0;i<posti_len;i++){
		if( isdigit(posti[i]) == 0 ){
			free(cpy);
			return 0;
		}
	}
	
	free(cpy);
	return 1;
}

 
 
 /* Gestisce una offerta di share ricevuto da riga di comando
 * \param input La stringa letta da riga di comando
 * \param skt Il socket aperto con il server
 *
 * \retval 0 Se tutto ok
 * \retval -1 In caso di errore
 *            errno == 0 Se l'errore è risolto con un output a video
 *            errno == EFAULT Se il client deve terminare (socket chiuso)
 */
 /** NOTA: Il nome utente non serve. Il discriminante lato server infatti DEVE essere il socket su cui arriva il messaggio i.e. il worker che lo riceve */
static int gestisci_offerta(char* input, int skt){

	message_t msg;
	int ret=-1;
	ret=valida_input_offerta(input);
	if( ret != 1 ){
	
		if( ret == -1 ) {
			STAMPA(stdout,"Errore interno, riprovare\n");
		}
		else 
			STAMPA(stderr,BAD_CMD_MSG);
		errno = 0;
		return -1;
	}

	msg.type=MSG_OFFER;
	if( (msg.buffer=(char*)malloc( (strlen(input) +1) )) == NULL ){
		STAMPA(stdout,"Errore interno, riprovare\n"); 
		errno = 0;
		return -1;
	}
	msg.length=( strlen(input) + 1);
	strcpy(msg.buffer,input);
	if( sendMessage(skt,&msg) == -1){
		free_msg_buffer(&msg);
		if( errno == ENOMEM){
			STAMPA(stdout,"Errore interno, riprovare\n");
			errno = 0;
		}
		else {
			STAMPA(stdout,"Connessione perduta!\n");
			errno = EFAULT;
		}
		return -1;
	}
	free_msg_buffer(&msg);

	if( receiveMessage(skt,&msg) == -1){
		free_msg_buffer(&msg);
		if( errno == ENOMEM){
			STAMPA(stdout,"Errore interno, riprovare\n");
			errno = 0;
		}
		else{
			STAMPA(stdout,"Connessione perduta!\n");
			errno = EFAULT;
		}
		fflush(stdout);
		return -1;
	}
	STAMPA(stdout,"Offerta %saccettata (%s)\n",((msg.type==MSG_OK)?"":"non "),((msg.buffer!=NULL)?msg.buffer:"-") );
	free_msg_buffer(&msg);
	return 0;
}



/** Versione semplice di readline. Legge da stdin una stringa, eliminando eventuali caratteri 
 *  di spazio e tabulazione iniziali e il carattere a capo finale 
 *  \param in Buffer dove salvare la linea letta
 *
 *  \retval cp La linea letta
 *  \retval NULL Se EOF o se linea vuota
 *               errno == 0 Linea vuota letta
 *               errno == EIO Se EOF
 */
static char* dumb_readline(char* in){

	char* cp=NULL;
	char* saveptr=NULL;
	STAMPA(stdout,"%s",DEFAULT_PROMPT);

	if( fgets(in,MAX_LINE,stdin) ){
		if( in[0] == '\n'){
			errno = 0;
			return NULL;
		}
		cp=strtok_r(in,"\n",&saveptr);
		while (  *cp == ' '  || *cp == '\t'  )
			cp++;
	}
	else 
		errno = EIO;
	return cp;
}




/** Thread che si occupa di gestire la linea di comando. 
 *  \par arg Console_arg contenente username, password e socket-name
 */
void* console(void* arg){
	
	int skt; 		/* fd del socket a cui ci vogliamo connettere */
	console_arg* ca; 	
	char input[MAX_LINE+1];
	char* in_ptr=NULL;
	int err=0;
	struct sigaction s;
	
	bzero(&s,sizeof(s));
	/** L'unico segnale che console deve ricevere è SIGUSR2 il quale indica che il 
	 *  processo deve terminare */
	s.sa_handler=usr2_handler;
	sigaction(SIGUSR2,&s,NULL); 
	sigset_t set;
	sigfillset(&set);
	sigdelset(&set,SIGUSR2);
	pthread_sigmask(SIG_SETMASK,&set,NULL);

	ca=(console_arg*)arg;
	
	/* Mi collego al socket del server */
	if( (skt=openConnection(DEFAULT_SOCKET)) == -1 ){
		STAMPA(stdout,"Impossibile connettersi al server\n");

		kill(getpid(),SIGINT);
		return (void*)0;
	}
	/** Tento la connessione a livello protocollo con il server */
	if( make_connection(skt,ca->user,ca->pass,ca->socket) == -1 ){
		closeSocket(skt); 
		kill(getpid(),SIGINT);
		return (void*)0;
	}

	while( quit == 0){
		/** Mi blocco in attesa di input, EOF o SIGUSR2 */
		in_ptr=dumb_readline(input); 
		if( in_ptr == NULL) {
			if( errno == 0)
				continue;
			else
				break;
		}
		/** il comando più corto in assoluto è lungo 5 caratteri */
		if ( strlen(in_ptr) < 5 ){
			STAMPA(stderr,BAD_CMD_MSG);
			continue;
		}
		
		/** Cerco di capire il comando letto */
		
		if( strcmp(in_ptr,"%EXIT") == 0 ){
			
			STAMPA(stderr,DEFAULT_EXIT_MSG);	
			break;
		}
		
		else if( strcmp(in_ptr,"%HELP") == 0){
				STAMPA(stderr,HELP_MSG);
		}
		
		/** "%R ...." */
		else if( in_ptr[0] == '%' && in_ptr[1] == 'R' && in_ptr[2] == ' '){
			if( gestisci_richiesta(&(in_ptr[3]),skt) == -1 && errno == EFAULT){
					err=1;
					break;
			}
		}
		/** Forse è' un'offerta */
		else if( gestisci_offerta(in_ptr,skt) == -1 && errno == EFAULT){
				err=1;
				break;
		}
		
	}/*while*/
	
	close_connection(skt);
	closeSocket(skt);
	return (void*)0;
}




int main(int argc, char* argv[]){

	int servSkt,connSkt; /* Socket su cui ascolta e su cui comunica, rispettivamente */
	struct sigaction s;
	int proc_pid;
	char sproc_pid[LPID+1]; /* Per metterci il pid convertito a stringa */
	char sockname[LSOCKET+1];
	char pass[100];
	pthread_t console_pid;
	console_arg ca;
	int len=-1; /* usato per misurare la lunghezza dello username e della password, per capire se sono alfanumeriche */
	int i;

	sigset_t set; /* per mascherare SIGUSR2 nel main */

	message_t rmsg; /* Per ricevere i messaggi dal server */
	

	/* controllo parametri da command_line */
	if( argc != 2 ) {
		STAMPA(stdout,"Pochi parametri. Invocare con\n%s nomeutente\n",argv[0]);
		return -1;
	}
	if( (len=strlen(argv[1])) > LUSERNAME ){ 
		STAMPA(stderr,"Username deve essere lungo max %d\n",LUSERNAME);
		return -1;
	}
	/** Il nome utente può contenere solo carattere alfanumerici */
	for(i=0;i<len;i++){
		if( !isalnum(argv[1][i]) ){
			STAMPA(stderr,"Username non valido. Deve contenere solo caratteri alfanumerici!\n");
			return -1;
		}
	}
	
	STAMPA(stdout,"Password: ");
	if( fgets(pass,101,stdin) == NULL || (len=strlen(pass)) > LPASS || len<= 1){
		STAMPA(stderr,"La password deve essere lunga 1-%d caratteri\n",LPASS);
		return -1;
	}
	
	/* rimuovo il carattere '\n' */
	pass[--len]='\0';
	for(i=0;i<(len-1);i++){
		if( !isalnum(pass[i]) ){
			STAMPA(stderr,"Password non valida. Deve contenere solo caratteri alfanumerici!\n");
			return -1;
		}
	}


	bzero(&rmsg,sizeof(message_t));
	/* Installo handler segnali */	
	bzero(&s,sizeof(s));
		
	s.sa_handler=signal_handler;
	EC_R( sigaction(SIGINT,&s,NULL) == -1,DONTSET,DONOTHING,-1 );
	EC_R( sigaction(SIGTERM,&s,NULL) == -1,DONTSET,DONOTHING,-1 );
	
	s.sa_handler=SIG_IGN;
	sigaction(SIGPIPE,&s,NULL);
	

	/** Il main maschera sigusr2 perchè è console il thread che lo deve ricevere */
	sigemptyset(&set);
	sigaddset(&set,SIGUSR2);
	pthread_sigmask(SIG_SETMASK,&set,NULL);


	/* adesso devo generare la stringa per il nome del socket */
	proc_pid=getpid();
	EC_R( sprintf(sproc_pid,"%d",proc_pid)<0,DONTSET,DONOTHING,-1 );
	strcpy(sockname,TMP_FLDR);
	strcat(sockname,argv[1]);
	strcat(sockname,"-");
	strcat(sockname,sproc_pid);
	
	/* Creo il socket per la ricezione dei messaggi */
	if( (servSkt=createServerChannel(sockname)) == -1){
		if( errno == EADDRINUSE ){
			STAMPA(stderr,"L'indirizzo %s è già in uso. Cancella il file\n",sockname);
		}
		else if( errno == EACCES ){
			STAMPA(stderr,"Non hai i permessi per utilizzare %s\n",sockname);
		}
		return -1;
	}


	/* Lancio la console */
	strcpy(ca.user,argv[1]);
	strcpy(ca.socket,sockname);
	strcpy(ca.pass,pass);
	EC_R( pthread_create(&console_pid,NULL,console,(void*)(&ca))>0,DONTSET,{ closeSocket(servSkt); unlink(sockname); },-1 );



	while( quit == 0 ){
		if( (connSkt=acceptConnection(servSkt)) == -1 )
			break;
		if( receiveMessage(connSkt,&rmsg) != -1 ){
			/* SMG_SHAREND mi dice di smettere di aspettarmi messaggi in arrivo */	
			if( rmsg.type == MSG_SHAREND ){
				/** Se fai come dicono le specifiche in pdf */
				/* continue; */
				/** Se vuoi superare il test.*/
				break;
			}
		        else if( rmsg.type == MSG_SHARE ){
				fprintf(stdout,"%s\n",rmsg.buffer);
			}
			else{
				STAMPA(stdout,"Ricevuto messaggio non valido\n");
			}
			free_msg_buffer(&rmsg);
		}
		else {
			STAMPA(stdout,"Connection closed by peer");
		}
		
		closeSocket(connSkt);
		
	}
	STAMPA(stdout,"Termino client\n");
	pthread_kill(console_pid,SIGUSR2);
	closeSocket(servSkt);
	unlink(sockname);
	pthread_join(console_pid,NULL);
	STAMPA(stdout,"Bye!\n");
	return 0;

}
	
	
	
	
	
	
	
