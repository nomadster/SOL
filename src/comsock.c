/**  \file 
 *   \brief implementazione libreria di comunicazione socket AF_UNIX
 *
 *    \author Alessandro Lensi - 297110
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "./common.h"
#include "./comsock.h"

/** -= CREATESERVERCHANNEL =- */
int createServerChannel(char* path){

	int s = -1;
	struct sockaddr_un sockaddr;

	EC_R( (path==NULL),EINVAL,DONOTHING,-1 );

	/** Attenzione, uso strlen! */
	EC_R( (strlen(path) > UNIX_PATH_MAX),E2BIG,DONOTHING,-1 );

	strcpy(sockaddr.sun_path, path);
	sockaddr.sun_family=AF_UNIX;

	EC_R( (s=socket(AF_UNIX,SOCK_STREAM,0))==-1,DONTSET,DONOTHING,-1 );
	
	EC_R( bind(s, (struct sockaddr*)&sockaddr,sizeof(sockaddr))==-1,errno,{ close(s); },-1 );

	EC_R( listen(s,COMSOCK_BACKLOG)==-1, errno, { close(s); }, -1 );

	return s;
		
}
/***********************************************/


/** -= CLOSESOCKET =- */
int closeSocket(int s){
	
	EC_R( shutdown(s,SHUT_RDWR)==-1,DONTSET,DONOTHING,-1 );
	return 0;
}
/***********************************************/


/** -= ACCEPTCONNECTION =- */
int acceptConnection(int s){
	return (accept(s,NULL,NULL));

}
/***********************************************/


/** -= RECEIVEMESSAGE =- */
int receiveMessage(int sc, message_t * msg){

	int nread=0;
	int lung=0;

	EC_R( msg==NULL,EINVAL,DONOTHING,-1 );


	nread=read(sc,&(msg->type),sizeof(char));
	EC_R( nread==-1,DONTSET,DONOTHING,-1 );
	EC_R( nread==0,EIO,DONOTHING,-1 );
	
	lung+=nread;
	nread=read(sc,&(msg->length),sizeof(unsigned int));
	EC_R(nread==-1 ,DONTSET,DONOTHING,-1 );
	EC_R(nread==0 ,EIO,DONOTHING,-1 );
	lung+=nread;

	if(msg->length != 0 ){
		EC_R( (msg->buffer=malloc(msg->length))==NULL,DONTSET,DONOTHING,-1 );
		
		nread=read(sc,msg->buffer,msg->length);
		EC_R( nread==-1 ,DONTSET,{free(msg->buffer);},-1 );
		EC_R( nread==0,EIO,{free(msg->buffer);},-1 );
		lung+=nread;
	}

	return lung;

}
/***********************************************/


/** -= SENDMESSAGE =- */

/** versione portabile di mempcpy(3) */
static void* my_mempcpy(void *dst, const void *src, size_t len) {
  return (void*)(((char*)memcpy(dst, src, len)) + len);
}


int sendMessage(int sc, message_t *msg){

	void* buff=NULL; /* per impacchettare il messaggio */
	size_t buff_size=0; /* la dimensione del buffer */
	void* last=NULL; /* per fare le copie consecutive */
	int retw=0; 

	EC_R( msg==NULL,EINVAL,DONOTHING,-1 );

	buff_size=(sizeof(char)+sizeof(unsigned int)+msg->length);

	EC_R( (buff=malloc(buff_size))==NULL,DONTSET,DONOTHING,-1 );

	/* copio i campi nel buffer */
	last=my_mempcpy(buff,&(msg->type),sizeof(char));
	last=my_mempcpy(last,&(msg->length),sizeof(unsigned int));

	if( msg->length != 0)
		last=my_mempcpy(last,msg->buffer,msg->length);

	EC_R( (retw=write(sc,buff,buff_size))==-1,DONTSET,{ free(buff);},-1 );
	free(buff);
	
	return retw;

}
/***********************************************/


/** -= OPENCONNECTION =- */
int openConnection(char* path){
	
	int fd=-1;
	int trial = NTRIALCONN;
	int check =-1; /* per il valore di ritorno di connect*/
	struct sockaddr_un sockaddr;
	
	EC_R( path==NULL,EINVAL,DONOTHING,-1 );

	EC_R( strlen(path)>UNIX_PATH_MAX,E2BIG,DONOTHING,-1 );

	EC_R( (fd=socket(AF_UNIX,SOCK_STREAM,0))==-1,DONTSET,DONOTHING,-1 );

	strcpy(sockaddr.sun_path, path);
	sockaddr.sun_family=AF_UNIX;

	while( (check=connect(fd,(struct sockaddr*)&sockaddr,sizeof(sockaddr))) == -1 && --trial != 0){
		if(errno==ENOENT || errno==EAGAIN)
			sleep(1);
		else
			break;
		errno=0;
	}

	/* controllo che connect() sia andata a buon fine */
	EC_R( check== -1,DONTSET, { close(fd); }, -1 );
		
	return fd;
}
/***********************************************/


/** -= FREE_MSG_BUFFER =- */
void free_msg_buffer(message_t* msg){
	if( msg->buffer != NULL)
		free(msg->buffer);
	msg->buffer=NULL;
	msg->length=0;
}

