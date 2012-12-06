/**  \file 
 *   \brief header libreria di gestione utenti
 *
 *    \author Alessandro Lensi - 297110
*/

#ifndef _HASHUTENTI_H
#define _HASHUTENTI_H


typedef struct el {
	/** Nome utente */
	char* username;
	/** Password */
	char* password;
	/** Nome del socket sul quale inviare i MSG_SHARE e MSG_SHAREND */
	char* socket;
	/** Flag per la verifica dello stato della connessione */
	int is_connected;
	struct el* next;
} user_t;


/** La hashtable :D */
typedef struct {
	user_t* head[N_HEAD];
} hashtable_t;



/** Inizializza la hashtable (ponendo a NULL tutte le head)
	\param ht La hashtable da inizializzare.

	\retval 0 Se tutto ok.
	\retval -1 In caso di errore -- setta errno
*/
int init_hashtable(hashtable_t* ht);

/** Consente l'inserimento di un nuovo utente.
	\param user L'utente da inserire
	\param ht L'hashtable dove inserirlo

	\retval 0 Se l'inserimento è andato a buon fine
	\retval -1 In caso di errore, ad esempio se è già presente l'utente user. setta errno.
		errno == EINVAL Parametri errati
		errno == EALREADY Utente già presente.
*/
int add_user(user_t* user, hashtable_t* ht);

/** Consente di verificare se esiste l'utente nella hashtable
 *  \param uname Il nome utente da ricercare
 *  \param ht L'hastable dove cercarlo

 *  \retval uu Puntatore all'utente, se esiste
 *  \retval NULL && errno==0 Se l'utente non esiste
 *  \retval NULL && errno!=0 In caso di errore
*/
user_t* user_exist(char* uname, hashtable_t* ht);


/** Consente di controllare la validità dei dati di accesso per un utente
	\param uu L'utente+password da verificare
	\param password La password da validare

	\retval 1 Se i dati corrispondono
	\retval 0 Se i dati NON corrispondono
	\retval -1 In caso di errore -- setta errno
*/
int check_password(user_t* uu, char* password);
/*int check_password(char* uu, char* password,hashtable_t* ht);*/
/* Controlla se l'utente è connesso al sistema.
 *  param uu Il nome utente da controllare
 *  param ht La hashtable su cui cercare
 *
 *  retval 1 Se l'utente è connesso
 *  retval 0 Se l'utente non è connesso
 *  retval -1 In caso di errore (setta errno)
 */
int is_connected(char* uu, hashtable_t* ht);

/** Consente di creare un nuovo user_t 
 *  \param uname Il nome utente
 *  \param pwd La password
 *  \param socket Il nome del socket dove è in attesa il client di questo utente
 *
 *  \retval uu Lo user_t creato
 *  \retval NULL in caso di errore, setta errno.
 */
user_t* create_user(char* uname, char* pwd, char* socket);

/** Consente di deallocare la struttura dati utente
 *  \param uu La struttura da deallocare -- pone (*uu)=NULL
 */
void free_user(user_t** uu);

/** Consente di deallocare tutta la hashtable
 *  \param ht La hashtable da deallocare
 */
void free_hashtable(hashtable_t* ht);

/** Aggiorna l'informazione sul socket per l'utente.
 * \param usr L'utente da aggiornare
 * \param skt Il nome della socket
 *
 * \retval 0 Se tutto ok
 * \retval -1 In caso di errore
 */
int update_user(user_t* usr, char* skt);

#endif
