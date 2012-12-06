/**
 * \file 
 * \brief Impementazione dei metodi per la gestione del grafo e degli share
 *
 * \author ALessandro Lensi - 297110
 */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "./common.h"
#include "./dgraph.h"

/* -= FREE_SHT =- */
/** Dealloca l'ABR.
 *  \param shT Puntatore al puntatore all'ABR.
 */
static void free_shT(share_t** shT){
    
    if( shT == NULL || (*shT)==NULL)
        return;
    free_shT( &((*shT)->left) );
    free_shT( &((*shT)->right) );
    free( (*shT)->id );
    free( (*shT) );
    (*shT) = NULL;
    return;
}
/***********************************************/

/* -= COPIA_SHT =- */

/** Crea una copia dell'ABR allocando le strutture dati
 *  \param shT Puntatore all'ABR da copiare
 *
 *  \retval shCp Copia dell'ABR
 *  \retval NULL in caso di errore (o se l'argomento era NULL :)
 *          errno == ENOMEM Se la malloc è fallita
 */

static share_t* copia_shT(share_t* shT){

	share_t* shCp = NULL;	
	if( shT == NULL ){
		errno = 0;
		return NULL;
	}
	
	EC_R( ((shCp=(share_t*)malloc(sizeof(share_t))) == NULL),
	       DONTSET,DONOTHING, NULL);
	
	EC_R( ((shCp->id=(char*)malloc(strlen(shT->id)+1)) == NULL),
	      DONTSET,{ free(shCp); }, NULL );
	
	strcpy(shCp->id,shT->id);
	shCp->posti=shT->posti;

	EC_R( (((shCp->left=copia_shT(shT->left)) == NULL) && (errno != 0)),
	      DONTSET, { free(shCp->id); free(shCp); }, NULL );
	
	EC_R( (((shCp->right=copia_shT(shT->right)) == NULL) && (errno != 0)),
	      DONTSET,{ free(shCp->id); free(shCp); }, NULL );

	errno = 0;
	return shCp;
}
/***********************************************/

/* -= STAMPA_SHT =- */

static void stampa_shT_recursive(share_t* shT){
	if( shT == NULL )
		return;
	stampa_shT_recursive(shT->left);
	fprintf(stdout,"\t\t ID:%s \t POSTI:%u\n",shT->id,shT->posti);
	stampa_shT_recursive(shT->right);
}
/** Stampa l'ABR in maniera ordinata
 * \param shT l'ABR da stampare
 */
static void stampa_shT(share_t* shT){
	if( shT == NULL)
		return;
		
	fprintf(stdout,"\t\t --------------------------\n");	
	stampa_shT_recursive(shT);	
	fprintf(stdout,"\t\t --------------------------\n");
	return;
}
/***********************************************/


/* -= NEW_GRAPH =- */
graph_t * new_graph(unsigned int n, char **lbl) {

    graph_t *ret = NULL;
    int i = 0;
    int lbl_len = 0; /* Per allocare i label */
    
    /* Controllo dei parametri. N.B. non voglio far creare grafi con 0 nodi */
    EC_R( (lbl == NULL || (*lbl) == NULL || n == 0),EINVAL,DONOTHING,NULL );
    

    /* Alloco il grafo*/
    EC_R( (ret = (graph_t*) malloc(sizeof (graph_t))) == NULL,
            DONTSET,
            DONOTHING,
            NULL
    );

    ret->node = NULL;
    ret->size = n;


    /* Alloco l'array di n>0 elementi*/
    EC_R( (ret->node = (node_t*) malloc(n * sizeof (node_t))) == NULL, 
          DONTSET,
          { free(ret); },
          NULL
    );

    for (i = 0; i < n; i++) {
        ret->node[i].label = NULL;
        ret->node[i].adj = NULL;
    }

    /* Adesso devo mettere ad ogni nodo una etichetta */
    for (i = 0; i < n; i++) {
        lbl_len=strlen(lbl[i]);
        EC_R(  ((ret->node[i].label = (char*) malloc((lbl_len + 1) * sizeof (char))) == NULL),
                DONTSET, 
                { free_graph(&ret); },
                NULL
        );
        strcpy((ret->node[i].label), lbl[i]);
    }
    return ret;
}
/***********************************************/


/* -= FREE_GRAPH  =- */
/** Dealloca la lista di adiacenza ponendo a NULL (*ee) */
static void free_adjL(edge_t** ee){
	if( ee == NULL || (*ee) == NULL )
		return;
	free_adjL(&((*ee)->next));

	free_shT( &((*ee)->share_tree) );

	free( (*ee) );
	(*ee)=NULL;
	return;
}

/** Dealloca tutti i campi del nodo puntato da nn */
static void free_nodo(node_t* nn){
	if( nn==NULL )
		return;
	free( nn->label );
	free_adjL( &(nn->adj) );
	return;
}

void free_graph(graph_t** g) {

    unsigned int i = 0;
 
    if (g == NULL || (*g) == NULL )
        return;

    for (i = 0; i < (*g)->size; i++)
    	free_nodo( &((*g)->node[i]) );

    /* elimino l'array dei nodi */
    free((*g)->node);
    (*g)->node = NULL;
 
    free((*g));
    (*g) = NULL;
}
/***********************************************/


/* -= PRINT_GRAPH =- */
/** Stampa la lista di adiacenza del nodo n */
static void stampa_adjL(edge_t* e, node_t* n){
	if( e == NULL || n == NULL)
		return;
	fprintf(stdout,"\t->%s costo: %.1f\n",n[e->label].label,e->km);

	fprintf(stdout,"%s",((e->share_tree==NULL)?"":"\t\tSHARE:\n"));
	stampa_shT(e->share_tree);

	stampa_adjL(e->next,n);

	return;
}

/** Stampa tutte le informazioni relative al nodo n nel grafo g */
static void stampa_nodo(graph_t* g, unsigned int n){
	
	if( (g == NULL) || (n > g->size) )
		return;

	fprintf(stdout,"\n%s (%d)\n", g->node[n].label, n);
	stampa_adjL(g->node[n].adj,g->node);
	return;
}


void print_graph(graph_t* g) {

    int i = 0;
    /* controllo parametri */
    if (g == NULL )
        return;

    fprintf(stdout,"******BEGIN-GRAFO*******\n\n");

    for (i = 0; i < g->size; i++){
	stampa_nodo(g,i);
    }
    fprintf(stdout,"******END-GRAFO*******\n\n");

    return;
}
/***********************************************/


/* -= COPY_GRAPH =- */
/** Crea una copia della lista di adiacenza, allocando le strutture dati necessarie
 *  \param adjL La lista da copiare
 *  
 *  \retval p Copia della lista di adiacenza
 *  \retval NULL in caso di errore.
 *	    Per una lista di valori di errno vedere man(3) malloc
 */
static edge_t* copia_adjL(edge_t* adjL) {

	edge_t *p = NULL; /* il puntatore da tornare */

	if( adjL == NULL ) {
		errno = 0;
		return NULL;
	}
	
	if( (p=malloc(sizeof(edge_t))) == NULL )
		return NULL; /* errno != 0 */

	p->label=adjL->label; /* sono due unsigned int */
	p->km=adjL->km;


	EC_R( (p->share_tree=copia_shT(adjL->share_tree)) == NULL && errno != 0,
	      DONTSET,{ free(p); },NULL );


	EC_R( (p->next = copia_adjL( adjL->next )) == NULL && errno != 0,
	      DONTSET,{ free(p); }, NULL );
	      
	return p;
}

/** Crea una copia del nodo, allocando le strutture dati necessarie
 *  \param src Il nodo da copiare
 *  \param dest Dove copiare tutti i dati ( deve essere allocato )
 * 
 *  \retval 0 Se l'operazione è andata a buon fine
 *  \retval -1 In caso di errore. 
 *             errno == EINVAL in caso di parametri errati
 *   	       Per altri valori di errno vedere man(3) malloc
 */
static int copia_nodo(node_t* src, node_t* dest){

	if( src == NULL || dest == NULL )
		return 0;
	
	if( (dest->label=malloc(sizeof(strlen(src->label)+1))) == NULL)
		return -1;
	
	strcpy(dest->label,src->label);

	EC_R( (dest->adj=copia_adjL(src->adj)) == NULL && errno != 0,DONTSET,
	      { free(dest->label); },-1 );
	
	return 0;
}
		

graph_t* copy_graph(graph_t* g) {

    graph_t *p = NULL;
    int i = 0; 
    int gsize = -1;

    EC_R( (g == NULL || g->size == 0 || (gsize = n_size(g)) == -1),
            EINVAL,
            DONOTHING,
            NULL
    );
    
    /* Alloco il grafo */
    EC_R( ((p = (graph_t*) malloc(sizeof(graph_t) ) ) == NULL),
            DONTSET,
            DONOTHING,
            NULL
    );

    p->node = NULL;
    p->size = gsize;

    /* alloco l'array di nodi */
    EC_R( ((p->node = (node_t*) malloc( gsize*sizeof(node_t) ) ) == NULL),
            DONTSET,
            { free(p); },
            NULL
    );

    /* copio i nodi */
    for (i = 0; i < gsize; i++) {
    
    	EC_R(   (copia_nodo(&(g->node[i]),&(p->node[i])) == -1 ),
    		DONTSET,
    		{ free_graph(&p); },
    		NULL 
    	);
    } /* fine for */

    return p;

}
/***********************************************/


/* -= ADD_EDGE =- */
static int valida_stringa_arco(char* ss);
int add_edge(graph_t * g, char* e) {

    char src_string[LLABEL + 1]; /* label nodo sorgente */
    int src_index = -1; /* indice del nodo sorgente */

    char dest_string[LLABEL + 1]; /* label nodo destinatario */
    int dest_index = -1; /* indice del nodo destinatario */

    char dist_string[LKM+1]; /* distanza in stringa */    
    double ddist = 0; /* distanza in double */
   
    edge_t *newEdge = NULL;

     /* controllo parametri */
    EC_R( (g == NULL || e == NULL || valida_stringa_arco(e) == -1),
            EINVAL,
            DONOTHING,
            -1
    );

    /* estraggo i dati dalla stringa in input */
    EC_R( (sscanf(e, "%[ A-Za-z]:%[ A-Za-z]:%[.0-9]\n", src_string, dest_string, dist_string) != 3),
            EINVAL,
            DONOTHING,
            -1
    );
    
    /** Se non esiste il nodo sorgente oppure non esiste quello destinatario ... */
     EC_R(  ((src_index = is_node(g, src_string))   == -1 ) ||
              ((dest_index = is_node(g, dest_string)) == -1 ),
             EINVAL,
             DONOTHING,
             -1
    );
    
    /** ... oppure esiste gi\a questo arco nel grafo ... */
    EC_R( (is_edge(g, src_index, dest_index)),EAGAIN,DONOTHING,-1 );

    /** ... oppure la stringa della distanza non è valida, segnalo ed esco */
    EC_R( ( (ddist = atof(dist_string)) < 0.0),EINVAL,DONOTHING,-1 );


    /** Creo il nuovo arco ed inizializzo i parametri, inserendo l'arco
      * nella lista di adiacenza del nodo con label src_index
      */
    EC_R( ((newEdge = malloc(sizeof (edge_t))) == NULL),DONTSET,DONOTHING,-1 );
    
    newEdge->km = ddist;
    newEdge->label = dest_index;
    
    newEdge->next = g->node[src_index].adj;
    g->node[src_index].adj = newEdge;


    newEdge->share_tree=NULL;

    
    return 0;

}
/***********************************************/


/* -= IS_NODE =- */
int is_node(graph_t* g, char* ss) {

    int i = 0;
    int gsize = -1;
    
    /* controllo parametri */
    EC_R( (g == NULL || ss == NULL || (gsize = n_size(g)) == -1) ||
           (valida_stringa_citta(ss) == -1),
            EINVAL,
            DONOTHING,
            -1
    );
    
    for(i=0;i<gsize;i++){

        if(g->node[i].label == NULL)
            continue;
        if (strcmp(g->node[i].label, ss) == 0)
            return i;
    }

    errno = 0;
    return -1;
}
/***********************************************/


/* -= IS_EDGE =- */
bool_t is_edge(graph_t* g, unsigned int n1, unsigned int n2) {

	if( g == NULL || n1 > g->size || n2 > g->size)
		return FALSE;

	return (trova_arco(n2,g->node[n1].adj) == NULL)?FALSE:TRUE;
}
/***********************************************/


/** -= DEGREE =- */
/** Conta il numero di elementi nella lista di adiacenza e
 *  \param e La lista di adiacenza
 *
 *  \retval n Numero di elementi nella lista di adiacenza
 */
static int conta_archi(edge_t* e){
	if( e == NULL )
		return 0;
	return (1 + conta_archi(e->next) );
}

int degree(graph_t * g, char* lbl) {

    int nodeIndex = -1; /* l'indice del nodo nel grafo */

    /* controllo parametri */
    EC_R( (g == NULL || lbl == NULL ||(nodeIndex = is_node(g, lbl)) == -1),
            EINVAL,
            DONOTHING,
            -1
    );

    return conta_archi(g->node[nodeIndex].adj);    
}
/***********************************************/


/* -= N_SIZE =- */
int n_size(graph_t* g) {

    int ret=-1;
    EC_R( (g == NULL),EINVAL,DONOTHING,-1 );
    
    errno=( ( (ret = (int)(g->size)) < 0 )? ENOTSUP : 0 );

    return ( (ret < 0)? -1 : ret );
}
/***********************************************/


/* -= E_SIZE =- */
int e_size(graph_t* g) {

    int i = 0;
    int gsize = -1; 
    int n = 0; 

    EC_R( (g == NULL || (gsize = n_size(g)) == -1 ),
            EINVAL,
            DONOTHING,
            -1
    );

    for (i = 0; i < gsize; i++) 
        n += conta_archi(g->node[i].adj);

    return n;
}
/***********************************************/


/* -= LOAD_GRAPH =- */
/** Valida il file delle citt\a tornando il numero di citt\a lette 
 *  se il file \e valido.
 *  \param ff Il file delle città
 *
 *  \retval n numero di città contenute nel file
 *  \retval -1 se si \e verificato un errore  
 *   errno == EINVAL se i parametri non sono validi
 *   errno == EILSEQ se il file contiene stringhe non valide
 *   errno == EIO se non è possibile leggere nel file
 */
static int valida_file_citta(FILE* ff) {
    
    int len = 0;
    int n = 0;

    char dump[LLABEL + 1];

    EC_R( (ff == NULL),EINVAL,DONOTHING,-1 );
      
    while ( fgets(dump, LLABEL + 2, ff) != NULL) {
        len=strlen(dump);
        /* Se non c'\e '\n' il file non \e corretto */
        EC_R( (dump[len-1] != '\n' ),EILSEQ,DONOTHING,-1 );
	dump[len-1]='\0';
	
	EC_R( (valida_stringa_citta(dump)==-1),DONTSET,DONOTHING,-1 );
             
        n++;
    }
    
    EC_R( (ferror(ff)),EIO, { clearerr(ff); rewind(ff); },-1 );
            
    clearerr(ff);
    rewind(ff);
    
    return n;

}


/** Valida la stringa in ingresso, tornando la sua lunghezza se valida.
 *  Una stringa valida \e una stringa con il formato"LBLCITTA1:LBLCITTA2:12.0"
 * \param ss La stringa da validare
 *
 * \retval Lunghezza della stringa (escluso il carattere di terminazione stringa finale)
 * \retval -1 altrimenti
 *   errno == EINVAL se i parametri non sono validi
 *   errno == EILSEQ Se la stringa contiene (sotto-)stringhe non valide
 */
static int valida_stringa_arco(char* ss) {

    int stop = LLABEL; 
    int len = 0;
    int i = 0;
    EC_R( (ss == NULL),
            EINVAL,
            DONOTHING,
            -1
    );
     
    /** Valido prima label citt\a */
    for (i = 0; (i < stop && ss[i] != ':'); i++) {
        EC_R( (!isalpha(ss[i]) && !isspace(ss[i])),
                EILSEQ,
                DONOTHING,
                -1
        );
    }
    EC_R( (i == LLABEL && ss[i] != ':'),EILSEQ,DONOTHING,-1 );

    i++; /* l'i-esimo carattere \e ':', incremento i */
        
    stop = i + LLABEL;
    /** Valido seconda label citt\a */
    for (; (i < stop && ss[i] != ':'); i++) {
        
        EC_R( (!isalpha(ss[i]) && !isspace(ss[i])),
                EILSEQ,
                DONOTHING,
                -1
        );
    }
    /* se i == stop (cioè quanti ne avevo letti prima + quanti ne potevo leggere ora)
     *  => ss[i] == ':' altrimenti la stringa non è valida
     * se i<stop e sono qui, vuol dire che ho letto : in ss[i];
     * questa parte della stringa è valida*/
    EC_R( (i == stop && ss[i] != ':'),
            EILSEQ,
            DONOTHING,
            -1
    );
    
    i++; /* l'i-esimo carattere \e ':', incremento i */
    
    stop = i + LKM - 2; 
    /** Valido la prima parte del numero che \e fatta al massimo da (LKM-2) cifre [0-9]*/
    for (; i < stop && ss[i] != '.'; i++) {
        EC_R( (!isdigit(ss[i])),
                EILSEQ,
                DONOTHING,
                -1
        );
    }

    /* a questo punto ss[i] deve essere '.' */
    EC_R((i == stop && ss[i] != '.'),
            EILSEQ,
            DONOTHING,
            -1
    );
  
   
    i++; /* l'i-esimo carattere \e '.', incremento i */

    /*adesso mi manca da leggere l'ultima cifra finale */
    EC_R( (!isdigit(ss[i])),
            EILSEQ,
            DONOTHING,
            -1
    );
  
    i++; /* l'ultimo carattere \e una cifra, incremento i */

    /* Ultimo controllo: non ci devono essere altri caratteri dopo questo */
    /* Mi affido al fatto che la stringa che mi passano sia terminata */
    len = strlen(ss);
    EC_R((i != len ),
            EILSEQ,
            DONOTHING,
            -1
    );

    return i;
}



/** ritorna il numero di archi dentro al file degli archi
    \param ff FILE* al file degli archi

    \retval n numero di archi contenuti nel file
    \retval -1 se si \e verificato un errore ad
     errno == EINVAL se i parametri non sono validi
     errno == EILSEQ se il file contiene stringhe non valide
     errno == EIO se non è possibile leggere nel file
*/

static int valida_file_archi(FILE* ff){


    int len = 0; /* lunghezza della stringa letta, per il check */
   
    int n= 0;
    char dump[LMAX_LABEL_NODI + 1];

    EC_R( (ff == NULL),EINVAL,DONOTHING,-1 );
    
    while ( fgets(dump, LMAX_LABEL_NODI + 2, ff) != NULL){

        len = strlen(dump);

        
        EC_R( (dump[len -1] != '\n'),
                EILSEQ,
                DONOTHING,
                -1
        );
        dump[len -1] = '\0';
        
        EC_R( ( valida_stringa_arco(dump) == -1),
                DONTSET,
                DONOTHING,
                -1
        );
        n++; /* letta una nuova stringa valida*/

    }
    
    EC_R( (ferror(ff)),
            EIO,
            { clearerr(ff); rewind(ff); },
            -1
    );

    clearerr(ff);
    rewind(ff);

    return n;
}

/** Carica i nodi nel FILE fdnodi in un nuovo grafo g.
 *  \param fdnodi Il file descriptor che contiene i nodi da aggiungere
 *  \param n_nodi Il numero di nodi che contiene
 *  
 *  \retval g Nuovo grafo creato
 *  \retval NULL In caso di errore
 *	         errno == EINVAL Parametri errati
 *		 errno == ENOMEM Malloc fallita
 *		 errno == EBADR Creazione grafo fallita
 */
static graph_t* carica_nodi( FILE* fdnodi, int n_nodi){
	
	int i;
	char** load; /* Per caricare le stringhe dal file fdnodes */
	bool_t merr=FALSE;
	char dump_city[LLABEL + 2]; /* 128char_validi + \n + \0*/
	graph_t* g=NULL;
	
	EC_R( (fdnodi == NULL) || (n_nodi <=0),EINVAL,DONOTHING,NULL );

	EC_R( ((load = (char**) malloc((n_nodi + 1) * sizeof (char*))) == NULL),
        	DONTSET,DONOTHING,NULL );

	/* Carico tutte le citt\a dentro load[] */
    	for (i = 0; i < n_nodi; i++) {
        	fgets(dump_city,LLABEL+2,fdnodi);
	        dump_city[ strlen(dump_city) - 1 ]= '\0';
        	if( (load[i] = (char*) malloc( strlen(dump_city) + 1 )) == NULL ){ 
        		merr = TRUE; 
        		break;
        	}
        	strcpy(load[i], dump_city);
    	}
    	
	load[i] = NULL;

	if( merr == FALSE )
		g = new_graph(n_nodi,load);
    	else{
    		for (--i; i >= 0; i--)
            		free(load[i]);
	        free(load);
	        errno = ENOMEM;
	        return NULL;
	}

        for(i=0;i<n_nodi;i++)
		free(load[i]);
	free(load);
	
	errno=((g==NULL)?EBADR:0);
	return g;

}


graph_t* load_graph(FILE * fdnodes, FILE * fdarcs) {

    graph_t* g = NULL; 
    int i;
    int cities = 0; /* il numero di città contenute nel file*/
    int edges = 0; /* il numero di archi contenuti nel file*/

    char dump_edge[LMAX_LABEL_NODI + 2 ]; /* LLABL+1+LLABEL+LKM+'\n'+'\0' */

    EC_R( (fdarcs == NULL || fdnodes == NULL),
            EINVAL,
            DONOTHING,
            NULL
    );


    /* valido il file degli archi e so quanti archi sono, se il file è corretto) */
    EC_R( ((edges = valida_file_archi(fdarcs)) == -1 ||
          (cities = valida_file_citta(fdnodes)) == -1),
            EINVAL,
            {rewind(fdarcs); rewind(fdnodes);},
            NULL
    );
    

    /* a questo punto so quanti nodi devo inserire nel mio grafo, li carico */
    EC_R( (g=carica_nodi(fdnodes,cities)) == NULL,DONTSET,DONOTHING,NULL );
    
    /* carico gli archi */
    for (i = 0; i < edges; i++) {
        fgets(dump_edge, LMAX_LABEL_NODI + 2, fdarcs);
        dump_edge[ strlen(dump_edge) - 1] = '\0';

        EC_R(  (add_edge(g, dump_edge) == -1),
                ECANCELED,
                { free_graph(&g); },
                NULL
        );
    }
    
    return g;
}
/***********************************************/


/* SAVE_GRAPH */
/** Salva la lista di adiacenza adjL, del nodo nd, appartenente al grafo g,
 *  nel file fdarcs 
 *  \param adjL La lista di adiacenza da salvare
 *  \param nd Il nodo di cui si vuol salvare la adjL
 *  \param na Array dei nodi del grafo
 *  \param fdarcs Il file su cui salvarla
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore 
 *             errno == EINVAL Parametri errati
 *             errno == EIO Errore scrittura su file
 */
static int salva_lista_adiacenza(edge_t* adjL, char* nd, node_t* na, FILE* fdarcs){

	EC_R( (nd==NULL) || (na==NULL) || (fdarcs == NULL),EINVAL,DONOTHING,-1 );

	if( adjL == NULL)
		return 0;
	EC_R( (fprintf(fdarcs, "%s:%s:%.1f\n", nd, na[adjL->label].label, adjL->km) < 0),
	      EIO, DONOTHING, -1);
	
	return salva_lista_adiacenza(adjL->next,nd,na,fdarcs);
}

/** Salva il nodo lbl nel fille fdnodes
 *  \param lbl L'etichetta del nodo da salvare
 *  \param fdnodes Il file su cui salvare il label
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore 
 *             errno == EINVAL Parametri errati
 *             errno == EIO Errore nella scrittura su file
 */
static int salva_nodo(char* lbl,FILE* fdnodes){	

	EC_R( (lbl==NULL) || (fdnodes == NULL),
	      EINVAL,DONOTHING,-1 );

	EC_R( (fprintf(fdnodes, "%s\n", lbl) < 0),
	      EIO, DONOTHING, -1 );

	return 0;

}

int save_graph(FILE * fdnodes, FILE * fdarcs, graph_t* g) {

    int nSize = 0; /* il numero di nodi del grafo */
    int i = 0; 

    EC_R( ( fdnodes == NULL || fdarcs == NULL || g == NULL || (nSize = n_size(g)) == -1),
            EINVAL,
            DONOTHING,
            -1
    );


    /* Inizio la scrittura di nodi ed archi */
    for (i = 0; i < nSize; i++) {

	EC_R( (salva_nodo(g->node[i].label,fdnodes) == -1), DONTSET, DONOTHING, -1 );
	
	EC_R( (salva_lista_adiacenza(g->node[i].adj,g->node[i].label,g->node,fdarcs) == -1),
	      DONTSET,DONOTHING,-1 );
    }
    return 0;
}
/***********************************************/

/** -= FUNZIONE DEFINITE DA ME PER PRIMO FRAMMENTO =- */

/* -= TROVA_ARCO = - */
edge_t* trova_arco(unsigned int lbl, edge_t* e){
	if( e == NULL )
		return NULL;
	if( e->label == lbl )
		return e;
	return trova_arco(lbl,e->next);
}
/***********************************************/


/* -= PRINT_WITH_DOT =- */

static void stampa_share_tree_dot(FILE*ff, share_t* sh){
	if(sh==NULL || ff == NULL)
		return;
	stampa_share_tree_dot(ff,sh->left);
	fprintf(ff,"%s:%d\\n",sh->id,sh->posti);
	stampa_share_tree_dot(ff,sh->right);
	return;
}


/** Stampa la lista di adiacenza del nodo id nel file ff
 *  \param id L'indice del nodo
 *  \param adj La lista di adiacenza del nodo
 *  \param ff IL file su cui stampare
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore
 *             errno == EINVAL parametri errari
 *             errno == EIO Errore di I/O
 */
static int stampa_lista_adiacenza_dot(int id, edge_t* adj, FILE* ff){

	EC_R( (ff==NULL),EINVAL,DONOTHING,-1 );
	if( adj == NULL )
		return 0;
	fprintf(ff,"%d -> %d [ style=\"bold\",label=\"costo:%.1f\\n",id,adj->label,adj->km);

	if( adj->share_tree != NULL){
		stampa_share_tree_dot(ff,adj->share_tree);
		fprintf(ff,"\" color=\"blue");
	}

	fprintf(ff,"\"];\n");
	return stampa_lista_adiacenza_dot(id,adj->next,ff);
}

	
/** Stampa il nodo sul file ff in formato dot
 *  \param id Indice del nodo nel grafo
 *  \param nn Il nodo da stampare
 *  \param ff Il file su cui stamparlo
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di errore
 *             errno == EINVAL parametri errari
 *             errno == EIO Errore di I/O
 */
static int stampa_nodo_dot(int id, node_t* nn, FILE* ff){

	EC_R( (nn==NULL) || (ff==NULL),EINVAL,DONOTHING,-1 );
	fprintf(ff,"%d [ style=\"bold\", label=\"%s\"",id,nn->label);
	fprintf(ff,"];\n");
	EC_R( (stampa_lista_adiacenza_dot(id,nn->adj,ff)==-1),
	       DONTSET,DONOTHING,-1 );
	return 0;
}


int print_with_dot(graph_t* g, const char* filename) {

    int i = 0;
    int gsize = -1;
    FILE* ff = NULL;
    
    /* controllo parametri */

    EC_R( ( g == NULL || 
            (gsize = n_size(g)) == -1 || 
            filename == NULL || 
            (ff = fopen(filename, "w")) == NULL),
            EINVAL,
            DONOTHING,
            -1
    );

    EC_R( (fprintf(ff, "digraph G {\n") < 0),
            EIO,
            DONOTHING,
            -1
    );
    
    
    for (i = 0; i < gsize; i++) {
    
    	EC_R( (stampa_nodo_dot(i,&(g->node[i]),ff) == -1) ,
    		DONTSET,DONOTHING,-1 );
    }
    fprintf(ff, "}\n");
    fclose(ff);

    return 0;

}
/***********************************************/


/* -= VALIDA_STRINGA_CITTA =- */
int valida_stringa_citta(char* ss){
	char cc[LLABEL+1];
	EC_R( (ss==NULL),EINVAL,DONOTHING,-1 );
	EC_R( (strlen(ss) > LLABEL),EINVAL,DONOTHING,-1 );
        /** La validazione avviene in due fasi.
          * La PRIMA, contrassegnata come *(1)* riconosce tutte le stringhe che
          * iniziano con caratteri NON AMMESSI.
          * La SECONDA, contrassegnata come *(2)* riconosce le stringhe che contengono
          * al loro interno caratteri non validi. Questo serve perchè sscanf di *(1)*
          * riconoscerebbe come valida la stringa "A 647328", che valida non \e, parsando solo "A".
          */ 
	EC_R( (sscanf(ss, "%[ A-Za-z]",cc) != 1),
                EILSEQ,
                DONOTHING,
                -1
        );
        
        EC_R( (strncmp(ss, cc, LLABEL + 1) != 0),
                EILSEQ,
                DONOTHING,
                -1
        );
	return 0;
}
/***********************************************/


/* -= FUNZIONI TERZO FRAMMENTO =- */


/* -= CREA_SHARE =- */
share_t* crea_share(char*id, int posti){

	share_t* nSh = NULL;
	EC_R( ((id==NULL) || (posti<=0)),EINVAL,DONOTHING,NULL );
	EC_R( (strlen(id) > LUSERNAME),EINVAL,DONOTHING,NULL );
	EC_R( (nSh=(share_t*)malloc( sizeof(share_t) )) == NULL,
	       DONTSET,DONOTHING,NULL );
	EC_R( (nSh->id=(char*)malloc( (strlen(id)+1) )) == NULL,
	       DONTSET, { free( nSh); },NULL);
	
	strcpy(nSh->id,id);
	nSh->posti=posti;
	nSh->left=nSh->right=NULL;
	return nSh;
}
/***********************************************/


/* -= AGGIUNGI_SHARE =- */	       
share_t* aggiungi_share(share_t* shT, share_t* who){

	int cmp = 0;
	if( shT==NULL)
		return who;
	/* Aggiungere uno share NULL o uno share che esiste già 
	 * non modifica l'abero */
	if( who == NULL || (cmp=strcmp(who->id,shT->id)) == 0 )
		return shT;

	if( cmp < 0 )
		shT->left = aggiungi_share(shT->left,who);
	else
		shT->right=aggiungi_share(shT->right,who);
	return shT;
}
/***********************************************/


/* -= TROVA_SHARE =- */
share_t* trova_share(share_t* shT, char* id){
	
	int cmp = 0;
	if( id == NULL)
		return NULL;
	
	if( shT == NULL)
		return NULL;
	if( (cmp = strcmp(id,shT->id)) == 0)
		return shT;
	else if ( cmp < 0 )
		return trova_share(shT->left,id);
	else
		return trova_share(shT->right,id);

}
/***********************************************/


/* -= TROVA_SHARE_LIBERO =- */
share_t* trova_share_libero(share_t* shT){
	
	share_t* sh = NULL;
	if( shT == NULL )
		return NULL;
	if( shT->posti > 0 )
		return shT;
	else if( (sh=trova_share_libero(shT->left)) != NULL)
		return sh;
	return trova_share_libero(shT->right);
}
/***********************************************/


/* -= RIMUOVI_SHARE =- */
static share_t* trova_min_right(share_t* sh){
	if( sh == NULL )
		return NULL;
	if( sh->left == NULL)
		return sh;
	return trova_min_right(sh->left);
}

share_t* rimuovi_share(share_t* shT, char* id){

	int cmp = 0;
	share_t* min_right=NULL;
	share_t* ret=NULL;
	EC_R( (id == NULL),EINVAL,DONOTHING,NULL );
	EC_R( (shT == NULL),ENODEV,DONOTHING,NULL );

	if( (cmp=strcmp(id,shT->id)) < 0)
		shT->left=rimuovi_share(shT->left,id);
	else if( cmp > 0 )
		shT->right=rimuovi_share(shT->right,id);
	
	else{
		if( (shT->left == NULL) && (shT->right==NULL) ){
			/* Foglia */
			free(shT->id);
			free(shT);
			return NULL;
		}
		else if( (shT->left != NULL) && (shT->right != NULL) ){
			/* Nodo con due figli */
			min_right=trova_min_right(shT->right);
			free(shT->id);
			shT->id=(char*)malloc(strlen(min_right->id)+1);
			strcpy(shT->id,min_right->id);
			shT->right=rimuovi_share(shT->right,shT->id); /* E' un nodo con al più un figlio */
			return shT;
		}
		else {
			free(shT->id);
			ret=((shT->left!=NULL)?shT->left:shT->right);
			free(shT);
			return ret;
		}

	}
	return shT;
}
/***********************************************/



