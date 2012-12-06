/**  \file 
 *   \brief Implementazione libreria di gestione utenti
 *
 *    \author Alessandro Lensi - 297110
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>

#include "./common.h"
#include "./hashutenti.h"

#ifndef ALT_HASH
/** Funzione hash. Controlla la prima lettera del nome utente e ritorna 
 *  il valore numerico corrispondente, da usare come indice nella
 *  hashtable. Cioè {A,a}->0,...,{Z,z}->25, tuttiglialtri->26
 *  \param c Il carattere da hashare
 *
 *  \retval N compreso in {0,...,26}
 */
static int hash(char c){
	
	if( isalpha(c) ){
		if(isupper(c) )
			return (c-'A');
		else
			return (c-'a');
	}
	return (N_HEAD-1);
}
#else
/** Funzione hash alternativa, abilitabile compilando con -DALT_HASH.
    Ritorna la somma dei caratteri che compongono il nome utente.
    \param str La stringa da hashare
    
    \retval ret La somma dei caratteri che compongono str
*/
static int hash(char* str){
	int ret=0;
   	int i=0;
   	if(str==NULL)
   		return 0;
   	while(str[i] != '\0' && i < LUSERNAME){
   		ret+=str[i];
   		i++;
   	}
   	return ret;
}
#endif

/** INIT_HASHTABLE **/
int init_hashtable(hashtable_t* ht){

	int i;
	EC_R( ht==NULL,EINVAL,DONOTHING,-1 );

	for(i=0;i<N_HEAD;i++)
		ht->head[i]=NULL;
	return 0;
}
/***********************************************/


/** ADD_USER **/
int add_user(user_t* user, hashtable_t* ht){

	int index=-1;
	
	EC_R( ( user==NULL || ht == NULL ),EINVAL,DONOTHING,-1 );
	EC_R( (user_exist(user->username,ht)) != NULL,EALREADY,DONOTHING,-1 );

	#ifndef ALT_HASH
	index=hash((user->username[0]));
	#else
	index=hash(user->username);
	index=index%N_HEAD;
	#endif
	user->next=ht->head[index];
	ht->head[index]=user;
	user->is_connected=1;
	return 0;
}
/***********************************************/


/* USER_EXIST */
user_t* user_exist(char* uname, hashtable_t* ht){

	int index=-1;
	user_t* scorr=NULL;
	EC_R( (uname==NULL||ht==NULL),EINVAL,DONOTHING,NULL );
	#ifndef ALT_HASH
	index=hash(uname[0]);
	#else
	index=hash(uname);
	index=index%N_HEAD;
	#endif
	scorr=ht->head[index];
	while(scorr!=NULL){
		if( strcmp(uname,scorr->username) == 0){
			return scorr;
		}
		scorr=scorr->next;
	}

	errno=0;
	return NULL;

}
/***********************************************/


/* CHECK_PASSWORD */
int check_password(user_t* uu, char* password){

	EC_R( (uu==NULL||password==NULL),EINVAL,DONOTHING,-1 );

	return ((strcmp(uu->password,password)==0)?1:0);

}
/*
int check_password(char* uu, char* password, hashtable_t* ht){
	user_t* cu=NULL;
	EC_R( (uu==NULL) || (password==NULL) || (ht==NULL),EINVAL,DONOTHING,-1 );
	EC_R( (cu=user_exist(uu,ht)) == NULL,EINVAL,DONOTHING,-1);
	return ((strcmp(cu->password,password)==0)?1:0);
}*/
/***********************************************/


/** -= IS_CONNECTED =- */
int is_connected(char* uu, hashtable_t* ht){

	user_t* usr=NULL;
	EC_R( uu==NULL || ht == NULL,EINVAL,DONOTHING,-1 );
	
	EC_R( (usr=user_exist(uu,ht))==NULL,0,DONOTHING,-1 );
	
	return (usr->is_connected);
}	
/***********************************************/


/* CREATE_USER */
user_t* create_user(char* uname, char* pwd, char* socket){

	user_t* uu=NULL;
	EC_R( (uname==NULL||pwd==NULL||socket==NULL),EINVAL,DONOTHING,NULL );
	
	
	if( (uu=(user_t*)malloc(sizeof(user_t)))==NULL ){
		return NULL;
	}
	uu->username=uu->password=uu->socket=NULL;

	if( (uu->username=(char*)malloc(strlen(uname)+1)) == NULL ||
	    (uu->password=(char*)malloc(strlen(pwd)+1)) == NULL || 
	    (uu->socket=(char*)malloc(strlen(socket)+1)) == NULL ){
	      free_user( &uu ); 
	      return NULL;
	}
	      
	(void)strcpy(uu->username,uname);
	(void)strcpy(uu->password,pwd);
	(void)strcpy(uu->socket,socket);
	uu->is_connected=0;
	return uu;

}


/***********************************************/

/** -= FREE_USER =- */
void free_user(user_t** uu){
	
	if( uu==NULL || (*uu) == NULL)
		return;

	if( (*uu)->username != NULL )
		free((*uu)->username);
	if( (*uu)->password != NULL )
		free( (*uu)->password);
	if( (*uu)->socket != NULL )
		free( (*uu)->socket);

	free( (*uu) );
	(*uu)=NULL;
	return;
}
/***********************************************/



/** -= FREE_HASHTABLE =- */
void free_hashtable(hashtable_t* ht){

	int i;
	user_t* scorr;
	if( ht==NULL  )
		return;

	for(i=0;i<N_HEAD;i++){
		scorr=ht->head[i];
		while( ht->head[i] != NULL){
			ht->head[i]=scorr->next;
			free_user(&scorr);
			scorr=ht->head[i];
		}
	}
	return;
}
/***********************************************/

/** -= UPDATE_USER =- */
int update_user(user_t *usr, char* skt){

	int len=0;
	EC_R( (usr == NULL || skt == NULL),EINVAL,DONOTHING,-1 );
	EC_R( (len=strlen(skt)) == 0,EINVAL,DONOTHING,-1 );
	free(usr->socket);
	EC_R( (usr->socket=malloc((len+1))) == NULL,DONTSET,DONOTHING,-1 );
	strcpy(usr->socket,skt);
	return 0;
}
/***********************************************/

