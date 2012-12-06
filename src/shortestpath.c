/**
 * \file 
 * \brief Impementazione delle funzioni per il calcolo dei cammini minimi su grafo diretto
 *
 * \author ALessandro Lensi - 297110
 */
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "./common.h"
#include "./dgraph.h"
#include "./shortestpath.h"

/* -= DIJKSTRA =- */
/** Indica se un dato qset è vuoto
 * \param qset Il qset da verificare
 * \param qset_size La dimensione del qset
 *
 * \retval TRUE se il qset è vuoto (NULL è un qset vuoto)
 * \retval FALSE Altrimenti
 */
static bool_t is_empty_set(bool_t *qset, int qset_size) {

    int i;
    if( (qset == NULL || qset_size <= 0) )
	return TRUE;

    for (i = 0; i < qset_size; i++)
        if (qset[i] == TRUE) /* non è vuoto*/
            return FALSE;

    return TRUE; /* sono tutti a FALSE, è vuoto*/
}

/** Torna l'indice del nodo con vettore delle distanze minore 
 *  \param qset Indica la presenza o meno nel qset
 *  \param dist Puntatore al vettore delle distanze
 *  \param dist_size La dimensione del vettore delle distanze
 *
 *  \retval index_of_min Indice del più vicino nodo
 *  \retval UNDEF Se non c'è un nodo più vicino
 *  \retval -1 In caso di errore
 *             errno == EINVAL Se parametri errati
*/
int select_smallest_dist(bool_t *qset, double *dist, int dist_size) {

    int i;
    int index_of_min = (int) UNDEF;
    double value_of_min = INFINITY;

    EC_R( (dist == NULL || qset == NULL || dist_size <= 0),
            EINVAL,
            DONOTHING,
            -1
    );

    /* inizio a cercare il nodo con valore dist minore*/
    for (i = 0; i < dist_size; i++) {

        if (qset[i] == TRUE) { /* considero solo i nodi nell'insieme Q*/
            if (value_of_min == INFINITY) { /* se value_of_min \e +oo allora qualsiasi nodo ha distanza <= di +oo*/
                value_of_min = dist[i];
                index_of_min = i;
            } else {
                if (dist[i] == INFINITY)
                    continue;

                if (dist[i] < value_of_min) {
                    value_of_min = dist[i];
                    index_of_min = i;
                }
            }
        } /* fine if (qset..)*/
    }

    return index_of_min;
}


double* dijkstra(graph_t* g, unsigned int source, unsigned int** pprec) {

    double *dist = NULL;

    int i = 0;
    int u = -1; /* il nodo che viene estratto dall'insieme Q nell'algoritmo di Dijkstra*/
    int v = 0; /* usato per aggiornare le distanze dei nodi adiacenti ad u*/
    int gsize = -1;
    double alt = -1.0;
    edge_t *scorr = NULL;
    bool_t *qset = NULL; /* l'insieme Q */
    bool_t have_pprec = FALSE; /* TRUE se pprec != NULL*/
    bool_t err = FALSE;


    if (pprec != NULL)
        have_pprec = TRUE;
						  /* Si può creare un grafo senza nodi? */
    EC_R( (g == NULL),EINVAL,DONOTHING,NULL);
    EC_R( ((gsize = n_size(g)) == -1),DONTSET,DONOTHING,NULL);
    EC_R( (gsize == 0 || gsize < source),EINVAL,DONOTHING,NULL);


    /* alloco l'array delle distanze */
    EC_R( ((dist = (double*) malloc(gsize * sizeof (double))) == NULL),
            DONTSET,
            DONOTHING,
            NULL
    );

    /* alloco l'insieme Q. Se qset[i] == TRUE vuol dire che quel nodo non è ancora stato esaminato*/
    EC_R( ((qset = malloc(gsize * sizeof (bool_t))) == NULL),
            DONTSET,
            { free(dist); },
            NULL
    );

    /* se ho pprec, alloco il vettore dei predecessori */
    if (have_pprec) {
        EC_R( (((*pprec) = (unsigned int *) malloc(gsize * sizeof (unsigned int))) == NULL),
                DONTSET,
                { free(qset); free(dist); },
                NULL
        );
    }


    /* la distanza dalla sorgente a tutti i nodi è +oo e tutti i nodi vanno messi in Q*/
    for (i = 0; i < gsize; i++) {
        dist[i] = INFINITY;
        qset[i] = TRUE;
        /* inizializzo i predecessori a UNDEF, se ho il vettore dei predecessori */
        if (have_pprec)
            (*pprec)[i] = UNDEF;
    }

    /* la distanza del nodo sorgente dal nodo sorgente è 0*/
    dist[source] = 0.0;
    while (!is_empty_set(qset, gsize) && errno == 0) {
        if ((u = select_smallest_dist(qset, dist, gsize)) == -1) {
            err = TRUE;
            break;
        }

        if (dist[u] == INFINITY) /* ci sono nodi che non sono raggiungibili, Dijkstra ha finito :D*/
            break;

        qset[u] = FALSE;
        scorr = g->node[u].adj;

        while (scorr != NULL) {
            v = scorr->label;
            alt = dist[u] + scorr->km;
            if (dist[v] == INFINITY) {
                dist[v] = alt;
                if (have_pprec)
                    (*pprec)[v] = u;
            } else if (alt < dist[v]) {
                dist[v] = alt;
                if (have_pprec)
                    (*pprec)[v] = u;
            }
            scorr = scorr->next;

        }
    } /* fine while()*/
    if (errno != 0) /* vuol dire che sono uscito dal while perchè errno != 0*/
        err = TRUE;
    
    
    /* qui devo deallocare tutto*/
    free(qset);

    if (err == TRUE) {
        free(dist);
        if (have_pprec) {
            free((*pprec));
            pprec = NULL;
        }
        dist = NULL;
        errno = EINVAL;
    }
    return dist;

}
/***********************************************/


/* -= SHPATH_TO_STRING =- */
char* shpath_to_string(graph_t * g, unsigned int n1, unsigned int n2, unsigned int * prec) {

    int gsize = -1; /* il numero di nodi nel grafo */
    unsigned int *rotta = NULL; /* l'array della rotta al contrario */
    char *ret; /* la stringa da tornare */
    int i = 0; /* l'indice dell'elemento corrente nell'array rotta*/
    unsigned int p; /* il nodo correntemete selezionato, di cui voglio sapere il predecessore */
    int max; /* lunghezza massima della stringa da tornare */
    int j = 0; /* per azzerare la stringa di ritorno */

    EC_R( (g == NULL || prec == NULL),EINVAL,DONOTHING,NULL);
    EC_R( ((gsize = n_size(g)) == -1 ),DONTSET,DONOTHING,NULL);
    EC_R( ( n1 > gsize || n2 > gsize),EINVAL,DONOTHING,NULL);
    
    /* la rotta da me a me non esiste :D*/
    if (n1 == n2) {
        errno = 0;
        return NULL;
    }

    /* creo un array di gsize elementi che mi serve per salvarmi gli indici dei nodi nella rotta da n2 ad n1
     * (infatti percorro il vettore dei predecessori da n2, la destinazione, ad n1, la sorgente, per stampare il cammino
     * di costo minimo da n1 a n2*/
    EC_R( ((rotta = (unsigned int*) malloc(gsize * sizeof (unsigned int))) == NULL),
            DONTSET,
            DONOTHING,
            NULL
    );

    /* il primo elemento in rotta[] \e l'ultimo del path*/
    rotta[0] = n2;
    i = 1;
    p = n2;

    /* qui tratto tutti i casi in cui c'è un predecessore ed è diverso dalla sorgente*/
    while (prec[p] != UNDEF && prec[p] != n1) {
        rotta[i] = prec[p]; /* metto il mio predecessore nella rotta inversa*/
        p = prec[p]; /* vediamo chi è il predecessore del mio predecessore */
        i++;
    }

    /* non esiste una rotta da n1 ad n2*/
    if (prec[p] == UNDEF){
    	free(rotta); 
    	return NULL;
    }

    if (prec[p] == n1) { /* sono arrivato alla sorgente*/
        rotta[i] = n1;
        i++;
    }

    /* alloco la stringa che al massimo è lunga
     * (i*LLABEL) per le etichette di nodi +
     * (i-1) per gli $ +
     * 1 per il '\0'
     * caratteri  */
    max = ((i * LLABEL)+(i - 1) + 1);

    EC_R( ((ret = (char*) malloc(max * sizeof (char))) == NULL),
            DONTSET,
            { free(rotta); },
            NULL
    );
            
    /* imposto tutta la stringa a '\0' */
    for(j=0;j<max;j++)
    		ret[j] = '\0';



    for (i = i - 1; i > 0; i--) {
        /* Controllo inconsistenze nel grafo di origine */
        EC_R( (g->node[rotta[i]].label == NULL),
                EINVAL,
                { free(rotta); },
                NULL
        );
        strncat(ret, g->node[ rotta[i] ].label, LLABEL);
        strncat(ret, "$", 1);
    }
    
    /* Grafo inconsistente? */
    EC_R( (g->node[rotta[i]].label == NULL),
            EINVAL,
            { free(rotta); },
            NULL
    );
    strncat(ret, g->node[ rotta[i] ].label, LLABEL);

    free(rotta);
    return ret;


}
/***********************************************/




/* -= FUNZIONI TERZO FRAMMENTO =- */


/* -= AGGIUNGI_SHARE_PATH =- */
/** Conta i nodi nello shpath.
 *  \param shpath Il cammino minimo nella forma source$...$dest
 *
 *  \retval n Il numero dei nodi
 *  \retval -1 In caso di errore
 *             errno == EINVAL Parametro errato
 */
static int numero_nodi_shpath(char* shpath){
	int i=0;
	int n=0;
	if( shpath == NULL)
		return -1;
	
	for(i=0;(shpath[i] != '\0');i++)
		if( shpath[i] == '$')
			n++;
	return ++n; /* L'ultimo nodo non ha il '$' */
}
	
	
	
/** In pratica devo:
    (1) Trovare tutte le liste di adiacenza coinvolte. (Tutti i nodi che sono primo estremo di un arco del path)
    (2) Perogni lista, vedere se esiste l'arco (this_node,next_node)
    (3) Se non c'è inserirlo
    (4) Inserire lo share nello share_tree di quel nodo
    
    (5) Ripristinare tutto se per qualche motivo fallisce qualche cosa
*/


/** Converte lo shpath in un array di stringhe di nodi. Tale array viene allocato dalla funzione.
 *  \param shpath Lo shortest path da convertire
 *  \param n Numero di NODI nel path
 *
 *  \retval ret Array di char* : ret[i]="LabelNodoi-esimo"
 *  \retval NULL In caso di errore
 *             errno == EINVAL Parametri errati
 *             errno == ENOMEM Malloca fallita
 */
static char** tokenizza_shpath(char* shpath, int n){

        char** ret = NULL; /** L'array di char* da tornare */
        char* copy=NULL; /** Per copiare lo shpath */
        char* dollar=NULL; /** Per inidividuare i '$' */
        char* substr=NULL;  /** Trova l'etichetta del nodo */
        int i;
        int error=0;
        EC_R( shpath == NULL || n <=0,EINVAL,DONOTHING,NULL );
        EC_R( (ret=malloc( n * sizeof(char*))) == NULL,DONTSET,DONOTHING,NULL);
        EC_R( (copy=malloc( strlen(shpath) +1 )) == NULL,DONTSET,{free(ret);},NULL);
	strcpy(copy,shpath);
	substr=copy;
	/** Per ogni label di nodo i in shpath creo un buffer in ret[i] e ci copio dentro il label */
        for(i=0;i<n;i++){ 
        	if( i==(n-1) ) { /* L'ultimo nodo non ha il '$' */
        		if( (ret[i]=malloc(strlen(substr)+1)) == NULL){
        			error=1;
        			break;
        		}
        		strcpy(ret[i],substr);
        	}
        	else {
        		if( (dollar=strchr(substr,'$')) == NULL){
        			error=1;
        			break;
        		}
        		dollar[0]='\0';
			if( (ret[i]=malloc(strlen(substr)+1)) == NULL ){
				error=1;
				break;
			}
			strcpy(ret[i],substr);
			substr=&(dollar[1]);
		}
	}

	if( error==1) {
		for(--i;i>=0;i--)
			free(ret[i]);
		free(ret);
		free(copy);
		ret=NULL;
	}
	free(copy);
	errno=((error==1)?ECANCELED:0);
	return ret;
}
	
int aggiungi_share_path( graph_t* g, char* source, char* dest, char* id, int posti){

	char* shpath=NULL;
	unsigned int* pprec=NULL;
	double* vdist=NULL;

	int ss, dd; /* Indici dei nodi nel grafo */
	int nNodi=-1; /* Numero di nodi nel path */
	char **arrayNodi = NULL; 
	int i,j;
	int h,k; /* Indici dei nodi dell'arco (h,k) */
	edge_t* ed = NULL; /* Puntatore all'arco che sto considerando */
	share_t* nShare = NULL; /* Per creare il nuovo share da aggiungere */

	bool_t err=FALSE;
	
	EC_R( (g==NULL) || (source==NULL) || (dest==NULL) || (id==NULL) || (posti <= 0),
	      EINVAL, DONOTHING, -1 );
	
	EC_R( (ss=is_node(g,source)) == -1 || (dd=is_node(g,dest)) == -1,
	       EINVAL,DONOTHING,-1 );
	
	/* Calcolo lo shortest path in g, se non esiste esco e segnalo. Se esiste lo converto a stringa	 */
	EC_R( (vdist=dijkstra(g,ss,&(pprec))) == NULL,DONTSET,DONOTHING,-1 );
	free(vdist);
	shpath=shpath_to_string(g,ss,dd,pprec);
	free(pprec);
	EC_R( shpath == NULL,EBADR,DONOTHING,-1 );
	
	
	/* Nel grafo esiste un cammino, ed è minimo, da source a dest. Devo aggiungere le offerte di share
	 * a tutti gli archi coinvolti nel cammino. Per farlo prima tokenizzo tutti i nodi */
	nNodi = numero_nodi_shpath(shpath);
	EC_R( (arrayNodi = tokenizza_shpath(shpath,nNodi)) == NULL,DONTSET,{ free(shpath); }, -1 );
	
	free(shpath);/* non mi serve più */
	
	for(i=0; i<(nNodi-1);i++){
		/* Trovo indici dei nodi dell'arco (h,k) */
		EC_( (h = is_node(g,arrayNodi[i])) == -1,DONTSET,{ err=TRUE; break; } );
		EC_( (k = is_node(g,arrayNodi[i+1])) ==-1,DONTSET,{ err=TRUE; break; } );
		EC_( (ed = trova_arco(k,g->node[h].adj)) == NULL,
			     DONTSET,{ err=TRUE; break; } );
		
		/* ed adesso punta all'arco (h,k) */
		/* Devo aggiungere lo share a patto che non esista già. Se esiste è un errore. */
		EC_( trova_share(ed->share_tree,id) != NULL ,ECANCELED,{ err=TRUE; break; } );
		
		/* creo lo share e lo aggiungo allo share_tree */
		EC_( (nShare= crea_share(id,posti)) == NULL,DONTSET,{err=TRUE; break; } );
		ed->share_tree = aggiungi_share(ed->share_tree,nShare);
	}
	
	if( err == TRUE ){
		for(j=0;j<i;j++){
			h = is_node(g,arrayNodi[j]);
			k = is_node(g,arrayNodi[j+1]);
			ed = trova_arco(k,g->node[h].adj);
			ed->share_tree = rimuovi_share(ed->share_tree,id);
		}
	}
	for(i=0;i<nNodi;i++)
		free(arrayNodi[i]);
	free(arrayNodi);

	return ((errno!=0)?-1:0);
	
}
/***********************************************/


/** -= TROVA_UN_PASSAGGIO =- */
/** Tokenizza le (n-1) offerte in un array di 3*(n-1) elementi.
 *  Per ogni i=0..(n-1), k=(2*i)
 *           ret[i+k] = Chi offre il passaggio su l'arco
 *           ret[i+k+1] = Primo estremo di quell'arco
 *           ret[i+k+2] = Secondo estremo di quell'arco
 *
 *  \param offerte Array di puntatori alle offerte (n-1) selezionate
 *  \param nodi Etichette degli n nodi del path
 *  \param n Numero di nodi nel path
 *
 *  \retval offerte_tkn Array di 3*(n-1) stringhe rappresentanti la tokenizzazione
 *                      delle (n-1) offerte.
 *  \retval NULL In caso di errore -- setta errno
 *  \post Se torna NULL la memoria è libera
 */
static char** tokenizza_offerte(share_t** offerte,char** nodi, int n){

	char** offerte_tkn=NULL; /* Lo uso per spezzare le offerte */
	int i,k;
	bool_t err = FALSE; /* segnala errori di memoria */
	int len=0;
	
	EC_R( (offerte==NULL) || (nodi==NULL) || (n<=1),EINVAL,DONOTHING,NULL );
	for(i=0;i<(n-1);i++){
		if(offerte[i] == NULL || nodi[i] == NULL){
			errno = EINVAL;
			return NULL;
		}
	}
			
	/* n nodi, (n-1) archi => 3*(n-1) offerte_tkn */
	EC_R( (offerte_tkn=(char**)malloc( (3*(n-1))*sizeof(char*))) == NULL,
	      DONTSET,DONOTHING,NULL );
	
	for(i=0;i<(3*(n-1));i++){
		offerte_tkn[i]=NULL;
	}
	
	
	/* Perogni i=0...(n-1), sia k=(2*i). 
	 * offerte_tkn[i+k]="Chi offre il passaggio", cioè offerte[i]->id
	 * offerte_tkn[i+k+1]="NodoSorgente", cioè nodi[i]
	 * offerte_tkn[i+k+2]="NodoDestinatario" cioè nodi[i+1]*/
	for(i=0;i<(n-1);i++){
		k=(2*i);
		
		/** offerte_tkn[i+k]="Chi offre il passaggio", cioè offerte[i]->id */
		EC_( (len=strlen(offerte[i]->id))==0,DONTSET,{ err=TRUE; break; } );
		EC_( (offerte_tkn[i+k] = malloc( len + 1 )) == NULL,
		     DONTSET,{ err=TRUE; break; } );
		strcpy(offerte_tkn[i+k],offerte[i]->id);

		/** offerte_tkn[i+k+1]="NodoSorgente", cioè nodi[i] */
		EC_( (len=strlen(nodi[i]))==0,DONTSET,{ free(offerte_tkn[i+k]); err=TRUE; break; } );
		EC_( (offerte_tkn[i+k+1] = malloc(len+1)) == NULL,DONTSET, { free(offerte_tkn[i+k]); err=TRUE; break; } );
		strcpy(offerte_tkn[i+k+1] ,nodi[i]);

		/** offerte_tkn[i+k+2]="NodoDestinatario" cioè nodi[i+1] */
		EC_( (len=strlen(nodi[i+1]))==0,DONTSET,{ free(offerte_tkn[i+k]); free(offerte_tkn[i+k+1]); err=TRUE; break; } );
		EC_( (offerte_tkn[i+k+2] = malloc(len+1)) == NULL,
		     DONTSET,{ free(offerte_tkn[i+k+1]); free(offerte_tkn[i+k]); err=TRUE; break; } );
		strcpy(offerte_tkn[i+k+2],nodi[i+1]);
	}
	/* In caso di erorre devo liberare la memoria */
	if ( err == TRUE ){
		for(--i; i>=0; i--) {
			k=2*i;
			free(offerte_tkn[i+k]);
			free(offerte_tkn[i+k+1]);
			free(offerte_tkn[i+k+2]);
		}
		free(offerte_tkn);
		offerte_tkn=NULL;
		errno=ENOMEM;
	}
	return offerte_tkn;
}

/** Trasforma le offerte tokenizzate in un array di stringhe pronte per essere usate nei MSG_SHARE
 *  \pre offerte_tkn è stato generato secondo il formato di tokenizza offerte()
 *  \param offerte_tkn L'array di 3*(n-1) elementi creato con tokenizza_offerte()
 *  \param n Numero di nodi nel path
 *  \param richiedente Chi sta richiedendo il passaggio
 *
 *  \retval ret Array di n stringhe che contiene tutte le i, con i al più (n-1),
 *              stringhe che servono per portare richiedente a destione. 
 *              Gli altri n-i elementi sono posti a NULL.
 *  \retval NULL in caso di errore -- setta errno
 *
 */
static char** trasforma_offerte(char** offerte_tkn, int n, char* richiedente){

	int i,j;
	bool_t err = FALSE; /* Errori di memoria */
	char** ret=NULL;
	int ret_idx = 0; /* Indice corrente di ret */
	int lun_ret_k = -1; /* Lunghetta totale i-esima stringa di ret */
	int ultimo_offerente=0; /* Indice dell'ultimo offerente */
		
	EC_R( (offerte_tkn==NULL) || (n<=1) || (richiedente==NULL),
	      EINVAL,DONOTHING,NULL );
	for(i=0;i<(3*(n-1));i++){
		EC_R( offerte_tkn[i] == NULL,EINVAL,DONOTHING,NULL );
	}
		

     	EC_R( (ret=(char**)malloc(n*sizeof(char*)))==NULL,DONTSET,DONOTHING,NULL );
     	for(i=0;i<n;i++)
     		ret[i] = NULL;

        for(i=0;i < (3*(n-1)); ){
        	ultimo_offerente=i;
        	/* Questo passaggio mi consente di capire per quante triple consecutive
        	 * l'offerente è lo stesso */
	   	while( (i < ((3*(n-1))-2)) && ((i+3) < ((3*(n-1))-1)) ){
	  		if( strcmp(offerte_tkn[i],offerte_tkn[i+3])==0 )
	   			i=i+3;
	   		else
	   			break;
	   	}
	   	
	   	/* Qui adesso i è l'indice dell'inizio dell'ultima tripla consecutiva
	   	 * il cui offerente è offerte_token[ultimo_offerente] */
	   	
	   	/* Posso creare la stringa da mettere in ret. Prima devo misurarne la lunghezza */
	   	lun_ret_k = (strlen(offerte_tkn[ultimo_offerente])+1);  /* +1 per "$" */
	   	lun_ret_k += (strlen(richiedente)+1); /* +1 è per "$" */
	   	
	   	/* Per ogni tripla, esclusa l'ultima, misuro la lunghezza del secondo campo 
	   	 * cioè dell'etichetta del nodo che è al primo estremo dell'arco */
	   	for(j=ultimo_offerente;j<i;j++){
	   		if( j % 3 == 1)
		   		lun_ret_k+=(strlen(offerte_tkn[j])+1); /* +1 per "$" */
		}
	    	
	    	/* Dell'ultima tripla invece voglio le etichette di tutti e due i nodi
	    	 * estremi dell'ultimo arco */
	    	lun_ret_k+=(strlen(offerte_tkn[j+1])+1); /* +1 per "$" */
	    	lun_ret_k+=(strlen(offerte_tkn[j+2]));
	    
		/* +1 per il \0 */
	        EC_( (ret[ret_idx] = malloc( (lun_ret_k+1) )) == NULL,
	             DONTSET,{ err = TRUE; break; } );
	    
	    	/* Adesso non mi resta che copiare/concatenare le varie stringhe in
	    	 * offerte_tkn per creare la stringa finale */
	        strcpy(ret[ret_idx],offerte_tkn[ultimo_offerente]);
		strcat(ret[ret_idx],"$");
		strcat(ret[ret_idx],richiedente);
		strcat(ret[ret_idx],"$");
		
		for(j=ultimo_offerente;j<i;j++){
	    		if( j % 3 == 1 ){
			    	strcat(ret[ret_idx],offerte_tkn[j]);
			    	strcat(ret[ret_idx],"$");
			}
	    	}
	    	strcat(ret[ret_idx],offerte_tkn[j+1]);
	        strcat(ret[ret_idx],"$");
	        strcat(ret[ret_idx],offerte_tkn[j+2]);
	        ret_idx++;
	    	i=i+3;
	}
	
	/* Controllo errori */
	if( err == TRUE){
		for(--ret_idx;ret_idx>=0;ret_idx--)
			free(ret[ret_idx]);
		
		free(ret);
		ret=NULL;
		errno=ENOMEM;
	}
	
     	return ret;
}


/** Decrementa i posti liberi degli share. Mantiene l'invariante ovvero
 *  perogni shares[i] share[i]->posti--; se shares[i]->posti == 0 rimuovi_share(shares[i])
 *  \param n Numero archi e/o share da trattare
 *  \param edges Gli archi da cui eliminare gli share (sono n-1)
 *  \param shares Gli share a cui devo decrementare i posti liberi (sono n-1)
 *
 *  \retval 0 Se tutto ok
 *  \retval -1 In caso di parametri errati (forse può essere void questa funzione)
 */
static int decrementa_posti_liberi(int n, edge_t** edges, share_t** shares){

	int i;
	EC_R( (edges==NULL) || (shares==NULL),
	      EINVAL,DONOTHING,-1 );
	for(i=0;i<n;i++){
		if(shares[i] == NULL || edges[i] == NULL ){
			errno = EINVAL;
			return -1;
		}
	}
	
	
	for(i=0;i<(n-1);i++){
		shares[i]->posti--;
		if( (shares[i]->posti) == 0 )
			edges[i]->share_tree = rimuovi_share(edges[i]->share_tree,shares[i]->id);
	}
	return 0;
}

char** trova_un_passaggio(graph_t* share, char* source, char* dest, char* richiedente){

	int ss,dd; /* indice di source e dest */
	double* dijk=NULL;
	unsigned int* pprec=NULL;
	char* shpath = NULL;
	int n = -1; /* Il numero di nodi che compone il path */
	char** nodi = NULL; /* Qui ci metto le etichette dei nodi */
	edge_t** edges = NULL; /* Array che contiene tutti gli archi del path */
	int h,k; /* Per avere gli indici dei nodi per aggiungere tutti gli archi 
	          * (h,k) del apth all'array degli archi */
	share_t** offerte = NULL; /* Array per salvarci le offerte di passaggio in ciascun arco */
	int i;
	char** offerte_tkn=NULL; /* per tokenizzare le offerte */
	char** ret=NULL;
	EC_R( share==NULL || source==NULL || dest==NULL || richiedente==NULL || strlen(richiedente) > LUSERNAME,
	      EINVAL,DONOTHING,NULL );
	
	EC_R( (ss=is_node(share,source)) == -1 || (dd=is_node(share,dest)) == -1,EINVAL,DONOTHING,NULL );



	EC_R( (dijk=dijkstra(share,ss,&pprec)) == NULL,DONTSET,DONOTHING,NULL );
	free(dijk);
	EC_R( (shpath=shpath_to_string(share,ss,dd,pprec)) == NULL,0,{ free(pprec);},NULL );
	free(pprec); /* non mi serve più */
	/* conto il numero di nodi nel path */
	
	EC_R( (n = numero_nodi_shpath(shpath)) == -1,DONTSET,{ free(shpath); },NULL );

	/* tokenizzo lo shpath in Nodi[] */	
	EC_R( (nodi = tokenizza_shpath(shpath,n)) == NULL,DONTSET,{free(shpath);},NULL );
	free(shpath);
		
	/* Creo l'array degli archi e trovo tutti gli archi, assegnandoli all'array */
	EC_R( (edges=(edge_t**)malloc( (n-1) * sizeof(edge_t*))) == NULL,
	      DONTSET,{ 
	      for(i=0;i<n;i++){ free(nodi[i]);}
	      free(nodi); }, NULL );
	
	for( i=0; i< (n-1); i++){
		h=is_node(share,nodi[i]);
		k=is_node(share,nodi[i+1]);
		edges[i] = trova_arco(k,(share->node[h].adj));
		if(edges[i] == NULL){
			free(edges);
			for(i=0;i<n;i++)
				free(nodi[i]);
			free(nodi);
			errno=ECANCELED;
			return NULL;
		}
	}


	/* A questo punto devo trovare (n-1) share negli archi del path e salvarmeli
	 * in un array di share_t* */
	if( (offerte=(share_t**)malloc((n-1)*sizeof(share_t*))) == NULL ){
		free(edges);
		for(i=0;i<n;i++)
			free(nodi[i]);
		free(nodi);
		return NULL;
	}
	for(i=0;i<(n-1);i++)
		offerte[i]=NULL;
	
	if( (offerte[0] = trova_share_libero(edges[0]->share_tree)) != NULL ){
		for( i=1;i<(n-1);i++) {
			/* Se è disponibile un passaggio da parte dell'utente che mi ha dato il passaggio
			 * durante l'arco precedente, lo preferisco. Altrimenti prendo il passaggio 
			 * da qualcun'altro */
			if( (offerte[i] = trova_share(edges[i]->share_tree,offerte[i-1]->id)) == NULL){
				if( (offerte[i]=trova_share_libero(edges[i]->share_tree)) == NULL){
					free(offerte);
					free(edges);
					for(i=0;i<n;i++)
						free(nodi[i]);
					free(nodi);
					errno=0;
					return NULL;
				}
			}
		}
	}
	else {
		free(offerte);
		free(edges);
		for(i=0;i<n;i++)
			free(nodi[i]);
		free(nodi);
		errno=0;
		return NULL;
	}
	
	/** Ho (n-1) puntatori a share_t che mi permettono di arrivare da source a dest 
	 *  Le converto in un array di stringhe con triple <offerte,nodo-i,nodo-j>*/
	if( (offerte_tkn = tokenizza_offerte(offerte,nodi,n)) == NULL){
		free(offerte);
		free(edges);
		for(i=0;i<n;i++)
			free(nodi[i]);
		free(nodi);
		errno=ECANCELED;
		return NULL;
	}
	for(i=0;i<n;i++)
			free(nodi[i]);
		free(nodi);
	
	/** E adesso converto l'array in un altro array che contiene le stringhe da tornare */
	if( (ret=trasforma_offerte(offerte_tkn,n,richiedente)) == NULL ){
		for(i=0;i<(3*(n-1));i++)
			if( offerte_tkn[i] != NULL )
				free(offerte_tkn[i]);
		free(offerte_tkn);
		free(offerte);
		free(edges);
		errno=ECANCELED;
		return NULL;
	}
	for(i=0;i<(3*(n-1));i++)
		if( offerte_tkn[i] != NULL )
			free(offerte_tkn[i]);
	free(offerte_tkn);
	
	/** Devo decrementare i posti liberi di ciascun offerente */
	if( decrementa_posti_liberi(n-1,edges,offerte) == -1){
		free(offerte);
		free(edges);
		i=0;
		while(ret[i]!=NULL)
			free(ret[i]);
		free(ret);
		errno=ECANCELED;
		return NULL;
	}
	free(edges);
	free(offerte);
	 
	return ret;	  
	 
}
/***********************************************/
