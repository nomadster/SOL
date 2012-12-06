/**  \file 
 *   \brief File che contiene macro di error_checking e definizioni utili un po' a tutto
 *
 *    \author Alessandro Lensi - 297110
*/
#ifndef _COMMON_H_
#define _COMMON_H_



/** -=Macro per il controllo degli errori=- */
#ifndef DONTSET
#define DONTSET errno
#endif

#ifndef NODOTHING
#define DONOTHING {}
#endif

/** Macro per il controllo degli errori. Se test è vero esegue what2do e setta errno a errnoval. */
#define EC_(test,errnoval,what2do){ \
		if( (test) ) {\
			{what2do;}\
			(errno) = (errnoval);\
		}\
	}\

/** Macro per il controllo degli errori, con ritorno. Se test è vero esegue what2do, setta errno a errnoval e torna 
    what2return */
#define EC_R(test,errnoval,what2do,what2return){ \
		if( (test) ) {\
			{what2do;}\
			(errno) = (errnoval);\
			return (what2return);\
		}\
	}\
/****************************************************/


/** -=Server e Client=- */

/** Numero massimo di linee leggibili da stdin */
#ifndef MAX_LINE
#define MAX_LINE 2048
#endif

/** Prompt di default per dumb_readline(). Viene usato se prompt==NULL */
#ifndef DEFAULT_PROMPT
#define DEFAULT_PROMPT "->"
#endif

/** Messaggio di uscita dalla shell. Viene stampato quando si legge EOF (^D) */
#ifndef DEFAULT_EXIT_MSG
#define DEFAULT_EXIT_MSG "\n** Chiudo console **\n"
#endif


/** Messaggio che viene stampato quando si immette un comando non valido */
#ifndef BAD_CMD_MSG
#define BAD_CMD_MSG "Comando non valido. Digita %%HELP per una lista di comandi validi\n"
#endif
	

/** Messaggio che viene stampato quando si digita "%HELP" */
#ifndef HELP_MSG
#define HELP_MSG "%%HELP\tStampa questo messaggio\n\
			   %%R SORGENTE:DESTINAZIONE\tRichiede un passaggio\n\
			   SORGENTE:DESTINAZIONE:nposti\tOffre un passaggio. nposti deve essere intero\n\n"
#endif


/** Versione di fprintf() per programmi multithread. Oltre alla stampa a video, esegue il flush dello stream 
    Se il programma non è in modalità verbosa, non fa niente */
#ifdef VERBOSE
	#define STAMPA(f,...){\
		fprintf(f,__VA_ARGS__);\
		fflush(f);\
		}
#else
	#define STAMPA(f,...){}
#endif

/** Lunghezza massima username. E' stata spostata qui, dal file comsock.h 
    perché serviva, oltre a definire la macro LPASS, ai files di libcars */
#ifndef LUSERNAME
#define LUSERNAME 25
#endif
/** Lunghezza massima password */
#define LPASS 		LUSERNAME
/** Lunghezza massima stringa del pid (per generare il nome del socket): "99999" */
#define LPID		5 
/** Prefisso del socket name */
#define TMP_FLDR	"./tmp/"
/** Lunghezza massima nome del socket: "./tmp/username-99999" */
#define LSOCKET		(6+LUSERNAME+1+LPID)



/** La lunghezza della coda di backlog del socket. */
#ifndef COMSOCK_BACKLOG
        #define COMSOCK_BACKLOG SOMAXCONN
#elif COMSOCK_BACKLOG > SOMAXCONN
        #undef COMSOCK_BACKLOG
        #define COMSOCK_BACKLOG SOMAXCONN
#elif COMSOCK_BACKLOG < 0
	#undef COMSOCK_BACKLOG
	#define COMSOCK_BACKLOG 0
#endif

/** Il nome di default per il socket del server */
#ifndef DEFAULT_SOCKET
#define DEFAULT_SOCKET "./tmp/cars.sck"
#endif


/** Il nome di default per il file di log del server */
#ifndef DEFAULT_LOG_FILE
#define DEFAULT_LOG_FILE "./mgcars.log"
#endif

/** Il numero massimo di worker attivi */
#ifndef MAX_ACTIVE_WORKER
#define MAX_ACTIVE_WORKER 5
#endif


/** La grandezza della tabella hash degli utenti 
    ALT_HASH è una macro che, se definita a tempo di compilazione
    utilizza una funzione hash alternativa nella hashtable degli utenti
*/
#ifndef N_HEAD
	#ifndef ALT_HASH
		#define N_HEAD 27
	#else
		#define N_HEAD 50 /** vedere relazione per fine-tuning*/
	#endif
#endif

#endif
