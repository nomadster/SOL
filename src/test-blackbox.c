/**  \file 
 *   \brief Test BlackBox di tutti i files implementati, per i quali i test forniti non offrono una copertura
 *   adeguata. tali files sono "dgraph.*", "hashutenti.*", "shortestpath.*".
 *   I test più approfonditi di client e server verranno effettuati da un altro file, se rimane tempo.
 *   Per effettuare questi test, si usano le specifiche nei vari headers.
 *
 *    \author Alessandro Lensi - 297110
*/
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "common.h"
#include "dgraph.h"
#include "shortestpath.h"
#include "hashutenti.h"


#define PUNTATORE 'A'
#define INTERO 'B'
#define BOOL 'C'
#define SHARE 'D'

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

static char* nomi[] = { "Luigi", "Mario", "Giovanni", "Antonio", "Filippo", "Zorro" };

static char* citta[] = {"PISA", "LUCCA", "SIENA", "LIVORNO", NULL };
	
static void m_perror(void* rst,char type,char* value,int line){
	#ifdef VERBOSE
	perror("errno meaning for this call");
	int* i=NULL;
	bool_t tt;
	share_t *s=NULL;
	switch(type){
	
		case PUNTATORE: 
			fprintf(stderr,"Got \"%s\", should be \"%s\". errno is \"%d\" (%d)\n",(char*)(rst),value,errno,line);
			break;
		case INTERO: 
			i=rst;
			fprintf(stderr,"Got \"%d\", should be \"%s\". errno is \"%d\" (%d)\n",(*i),value,errno,line);
			break;
		case BOOL:
			tt=(*(bool_t*)(rst));
			fprintf(stderr,"Got \"%s\", should be \"%s\". errno is \"%d\" (%d)\n",((tt==TRUE)?"TRUE":"FALSE"),value,errno,line);
			break;
		case SHARE:
			s=(share_t*)(rst);
			fprintf(stderr,"Got \"%s\", should be \"%s\". errno is \"%d\" (%d)\n",((s==NULL)?"(null)":s->id),value,errno,line);
			break;
		default:
			break;
	}
	fprintf(stderr,"\n\n");
	errno=0;
	#endif
	return;
} 

int main(void){

	/******DGRAPH.H TEST******/
	graph_t* g=NULL,*h=NULL;
	int ret=-1;	
	bool_t tt;
	FILE *c=NULL,*d=NULL;
	edge_t *e=NULL;
	share_t* s=NULL, *s1=NULL;
	int i=0;
	double *dd=NULL;
	unsigned int* u=NULL;
	char* chr=NULL;
	char** pas=NULL;

/** new_graph */

	assert( (g=new_graph(1,NULL)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	assert( (g=new_graph(0,NULL)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);

	assert( (g=new_graph(0,citta)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	
	/* questa chiamata è sbagliata ma funziona lo stesso, creando un grafo con meno nodi
	   rispetto all'array dei label. Tutto ciò è corretto */
	assert( (g=new_graph(1,citta)) != NULL );
	m_perror(g,PUNTATORE,"NOT (null)",__LINE__);
	
	free_graph(&g);
	assert( g == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	g=new_graph(4,citta);
	assert( (h=copy_graph(NULL)) == NULL);
	m_perror(h,PUNTATORE,"(null)",__LINE__);
	
	assert( (h=copy_graph(g)) != NULL );
	m_perror(h,PUNTATORE,"NOT (null)",__LINE__);

	assert( (ret=add_edge(h,NULL)) == -1);
	m_perror(&ret,INTERO,"-1",__LINE__);

	assert( (ret=add_edge(h,"PISA:LUCCA:20.1")) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);

	assert( (ret=add_edge(h,"PISA:LUCCA:20.1")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);

	assert( (ret=add_edge(h,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=add_edge(NULL,NULL)) == -1);
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=add_edge(h,"COMO:MILANO:9.9")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=add_edge(h,"COMO:MILANO:e^ipi")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_node(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_node(h,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_node(h,"Banana")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_node(h,"PISA")) > -1 );
	m_perror(&ret,INTERO,"> -1",__LINE__);
	
	assert( (tt=is_edge(NULL,0,0)) == FALSE );
	m_perror(&tt,BOOL,"FALSE",__LINE__);
	
	assert( (tt=is_edge(h,0,0)) == FALSE );
	m_perror(&tt,BOOL,"FALSE",__LINE__);
	
	assert( (tt=is_edge(h,42,43)) == FALSE );
	m_perror(&tt,BOOL,"FALSE",__LINE__);
	
	assert( (tt=is_edge(h,is_node(h,"PISA"),is_node(h,"LUCCA"))) == TRUE );
	m_perror(&tt,BOOL,"TRUE",__LINE__);
	
	assert( (ret=degree(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=degree(h,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=degree(h,"COMO")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=degree(h,"LIVORNO")) > -1 );
	m_perror(&ret,INTERO,"> -1",__LINE__);
	
	assert( (ret=degree(h,"PISA")) > -1 );
	m_perror(&ret,INTERO,"> -1",__LINE__);
	
	assert( (ret=n_size(NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=n_size(h)) > 0 );
	m_perror(&ret,INTERO,"> 0",__LINE__);
	
	assert( (ret=e_size(NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=e_size(h)) > 0 );
	m_perror(&ret,INTERO,"> 0",__LINE__);
	
	free_graph(&h);
	assert( h == NULL );
	m_perror(h,PUNTATORE,"(null)",__LINE__);	
	
	c=fopen("./CITY.txt","r");
	d=fopen("./TUSCANY.map","r");
	
	assert( (g=load_graph(NULL,NULL)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	assert( (g=load_graph(c,NULL)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	assert( (g=load_graph(d,c)) == NULL );
	m_perror(g,PUNTATORE,"(null)",__LINE__);
	
	assert( (g=load_graph(c,d)) != NULL );
	m_perror(g,PUNTATORE,"NOT (null)",__LINE__);
	
	fclose(c);
	fclose(d);
	
	c=fopen("./tmp/cc.txt","w");
	d=fopen("./tmp/dd.map","w");
	
	assert( (ret=save_graph(NULL,NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=save_graph(c,NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=save_graph(c,d,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=save_graph(c,d,g)) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
	
	fclose(c);
	fclose(d);

	assert( (e=trova_arco(42,NULL)) == NULL );
	m_perror(e,PUNTATORE,"(null)",__LINE__);
	
	assert( (e=trova_arco(0,NULL)) == NULL );
	m_perror(e,PUNTATORE,"(null)",__LINE__);
	
	assert( (e=trova_arco(is_node(g,"LUCCA"),g->node[is_node(g,"PISA")].adj)) != NULL );
	m_perror(e,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (ret=print_with_dot(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=print_with_dot(g,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=print_with_dot(g,"/root/nonpuoiscriverequi") ) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=print_with_dot(g,"./tmp/grafo.dot")) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
	
	assert( (ret=valida_stringa_citta(NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=valida_stringa_citta("bcbasuy4738oqfgew8f747y738")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=valida_stringa_citta("PISALUadghsjagshjdgsahjdghsajgdhasghdgashjgdjashgdhsajgdhjasgdhjasgdhjsagdhasgdhjsaghdgashjdghjsagdhjsagdhjaCCAMASSALIVORNOSPEZIACARRARAPISAJSJAKJSAKASJAKSJAKSJAKSJAK")) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=valida_stringa_citta("PISA")) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
		
	assert( (s=crea_share(NULL,-1)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
		
	assert( (s=crea_share("Pippo",-1)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s=crea_share("PIppo",0)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s=crea_share("cnsdfhdusihfaiudsofdsfdsafdsfsafdsfsfhduaishfduishfuidohfdsuihdsiu",4)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s1=aggiungi_share(NULL,NULL)) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	assert( (s1=aggiungi_share(s1,NULL)) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	s1=NULL;
	
	fprintf(stderr,"---------------\n");
	for( i=0; i<6; i++){
		assert( (s=crea_share(nomi[i],3)) != NULL );
		m_perror(s,SHARE,nomi[i],__LINE__);
		assert( (s1=aggiungi_share(s1,s)) != NULL );
		m_perror(s1,SHARE,"NOT (null)",__LINE__);
	
	}
	fprintf(stderr,"---------------\n");
	
	s=NULL;
	assert( (s=trova_share(s1,NULL)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s=trova_share(NULL,NULL)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s=trova_share(s1,"giovani")) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	fprintf(stderr,"---------------\n");
	for(i=0;i<6;i++){
		assert( (s=trova_share(s1,nomi[i])) != NULL );
		m_perror(s,SHARE,nomi[i],__LINE__);
	}
	fprintf(stderr,"---------------\n");
	
	s=NULL;
	assert( (s=trova_share_libero(NULL)) == NULL );
	m_perror(s,SHARE,"(null)",__LINE__);
	
	assert( (s=trova_share_libero(s1)) != NULL );
	m_perror(s,SHARE,"Luigi",__LINE__);
	
	assert( (s1=rimuovi_share(NULL,NULL)) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	assert( (s1=rimuovi_share(s1,NULL)) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	assert( (s1=rimuovi_share(s1,"Plutonio")) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	fprintf(stderr,"---------------\n");
	for(i=0;i<5;i++){
		assert( (s1=rimuovi_share(s1,nomi[i])) == NULL );
		m_perror(s1,SHARE,nomi[i],__LINE__);
	}
	fprintf(stderr,"---------------\n");
	
	assert( (s1=rimuovi_share(s1,nomi[i])) == NULL );
	m_perror(s1,SHARE,"(null)",__LINE__);
	
	
	/*shortestpath.h*/
	assert( (dd=dijkstra(NULL,0,NULL)) == NULL );
	m_perror(dd,PUNTATORE,"(null)",__LINE__);
	
	assert( (dd=dijkstra(g,3000,NULL)) == NULL );
	m_perror(dd,PUNTATORE,"(null)",__LINE__);
	
	assert( (dd=dijkstra(g,0,NULL)) != NULL );
	m_perror(dd,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (dd=dijkstra(g,is_node(g,"PISA"),&u)) != NULL );
	m_perror(dd,PUNTATORE,"NOT (null)",__LINE__);
	m_perror(u,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (chr=shpath_to_string(NULL,99,100,NULL)) == NULL );
	m_perror(chr,PUNTATORE,"(null)",__LINE__);
		
	assert( (chr=shpath_to_string(g,99,100,NULL)) == NULL );
	m_perror(chr,PUNTATORE,"(null)",__LINE__);
	
	assert( (chr=shpath_to_string(g,99,100,u)) == NULL );
	m_perror(chr,PUNTATORE,"(null)",__LINE__);
	
	assert( (chr=shpath_to_string(g,is_node(g,"GROSSETO"),is_node(g,"GROSSETO"),u)) == NULL );
	m_perror(chr,PUNTATORE,"(null)",__LINE__);
	
	
	assert( (chr=shpath_to_string(g,is_node(g,"PISA"),is_node(g,"BARBERINO DI MUGELLO"),u)) != NULL );
	m_perror(chr,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (ret=aggiungi_share_path(NULL,NULL,NULL,NULL,-8)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,NULL,NULL,NULL,-8)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA",NULL,NULL,-8)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BARBERINO DI MUGELLO",NULL,-8)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BARBERINO DI MUGELLO","Pietro",-8)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BARBERINO DI MUGELLO","Pietro",0)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BARBERINO DI MUGELLO","Bahudis8932rhf38ohc2784h5f24578f7h724hf",1)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PI123SA","BARBERINO DI MUGELLO","Pietro",2)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BAR43f34BERINO DI MUGELLO","Pietro",2)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=aggiungi_share_path(g,"PISA","BARBERINO DI MUGELLO","Pietro",2)) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);

	assert( (pas=trova_un_passaggio(NULL,NULL,NULL,NULL)) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,NULL,NULL,NULL)) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISA",NULL,NULL)) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISA","LUCCA",NULL)) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISA","LUCCA","Filipphfudifher9h9hver9vh89ervh98dfvhdf89vdfh89o")) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISA","NONESISTE","Filippo")) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"NONESISTE","LUCCA","Filippo")) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISTOIA","ALTOPASCIO","Filippo")) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"PISA","BARBERINO DI MUGELLO","Filippo")) != NULL);
	m_perror(pas,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"ALTOPASCIO","FIRENZE","Luca")) != NULL);
	m_perror(pas,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (pas=trova_un_passaggio(g,"ALTOPASCIO","FIRENZE","Antonello")) == NULL);
	m_perror(pas,PUNTATORE,"(null)",__LINE__);
	
	free_graph(&g);
	assert( g==NULL );
	
/** hashutenti */
	hashtable_t ht;

	assert( (ret=init_hashtable(NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=init_hashtable(&ht)) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);

	user_t* us=NULL;
	
	user_t* create_user(char* uname, char* pwd, char* socket);
	assert( (us=create_user(NULL,NULL,NULL)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=create_user("Pipo",NULL,NULL)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=create_user("Pipo","Pupo",NULL)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=create_user("Pipo","Pupo","Socket")) != NULL );
	m_perror(us,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (ret=add_user(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=add_user(us,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=add_user(us,&ht)) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
	
	assert( (ret=add_user(us,&ht)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	us=NULL;
	assert( (us=user_exist(NULL,NULL)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=user_exist("Pipo",NULL)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=user_exist("Mario",&ht)) == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	assert( (us=user_exist("Pipo",&ht)) != NULL );
	m_perror(us,PUNTATORE,"NOT (null)",__LINE__);
	
	assert( (ret=check_password(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=check_password(us,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=check_password(us,"Passwordsbagliata")) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
	
	assert( (ret=check_password(us,"Pupo")) == 1 );
	m_perror(&ret,INTERO,"1",__LINE__);
	
	us->is_connected=0;
	assert( (ret=is_connected(NULL,NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_connected("Pipo",NULL)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_connected("NonEsiste",&ht)) == -1 );
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=is_connected("Pipo",&ht)) == 0 );
	m_perror(&ret,INTERO,"0",__LINE__);
	
	us->is_connected=1;
	assert( (ret=is_connected("Pipo",&ht)) == 1 );
	m_perror(&ret,INTERO,"1",__LINE__);
	
	us=create_user("Pipo","Pupo","Socket");
	free_user(&us);
	free_user(NULL);
	assert( us == NULL );
	m_perror(us,PUNTATORE,"(null)",__LINE__);
	
	fprintf(stderr,"---------------\n");
	for(i=0;i<6;i++){
		us=create_user(nomi[i],"Password","Socket");
		add_user(us,&ht);
	}
	fprintf(stderr,"---------------\n");
	
	assert( (ret=update_user(NULL,NULL)) == -1);
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	assert( (ret=update_user(us,NULL)) == -1);
	m_perror(&ret,INTERO,"-1",__LINE__);
	
	fprintf(stderr,"---------------\n");
	for(i=0;i<6;i++){
		us=user_exist(nomi[i],&ht);
		assert( (ret=update_user(us,"NuovoSocket")) == 0 );
		m_perror(&ret,INTERO,"0",__LINE__);
	}	
	fprintf(stderr,"---------------\n");
	
	free_hashtable(NULL);
	free_hashtable(&ht);
	
	fprintf(stderr,"---------------\n");
	for(i=0;i<N_HEAD;i++){
		assert( (ht.head[i]) == NULL );
		m_perror( (ht.head[i]),PUNTATORE,"(null)",__LINE__);
	}
	fprintf(stderr,"---------------\n");
	return 0;
}
