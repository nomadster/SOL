/**
  \file 
  \brief definizione dei prototipi del grafo rappresentato come array 0..N-1

 \author lso11 + Alessandro Lensi - 297110 
*/

#ifndef __DGRAPH_H
#define __DGRAPH_H
/** Rappresenta una offerta di share */
typedef struct share {
    /** Il nome utente di chi offre lo share */
    char* id;
    /** I posti residui */
    unsigned int posti;
    /** Foglia sinistra */
    struct share* left;
    /** Foglia destra */
    struct share* right;
} share_t;


/** tipo enumerato booleano vero/falso */
typedef enum bool {TRUE=1, FALSE=0} bool_t;

/** elemento lista di adiacenza */
typedef struct edge {
  /** etichetta nodo adiacente (indice nell'array) */
  unsigned int label;     
  /** peso dell'arco (distanza in km) */
  double km;      
  /** prossima adiacenza*/
  struct edge * next;    

  /** La ABR degli share per questo arco */
  share_t* share_tree;

} edge_t;

/** lunghezza massima label -- in caratteri */
#define LLABEL 128
/** lunghezza massima distanza in km -- in caratteri */
#define LKM 32

/** lunghezza massima della stringa che descrive gli archi -- in caratteri*/
#define LMAX_LABEL_NODI LLABEL + 1 + LLABEL + 1 + LKM

/** nodo del grafo */
typedef struct node {
/** etichetta informativa (nome citta) */
    char* label;      
/** lista di adiacenza */
    edge_t * adj;          
} node_t;

/** Grafo orientato rappresentato come array 0 .. (N-1) di vertici
*/
typedef struct graph {
  /** array dei  nodi */
  node_t * node;  
  /** numero nodi */
  unsigned int size;
} graph_t;



/** crea un grafo 
 *   \param n numero dei nodi del grafo
 *   \param lbl array di etichette de nodi formato:
 *   static const char* citta[] = {
 *       "PISA",
 *       "LUCCA",
 *       "SIENA",
 *       "LIVORNO",
 *       NULL,
 *      };
 *
 *   Attenzione: le citta possono contenere solo caratteri alfanumerici e lo spazio ' '
 *
 *   \retval p puntatore al nuovo grafo (se tutto e' andato bene)
 *   \retval NULL se si e' verificato un errore
 *                errno == EINVAL Parametri errati
 *   		  Per altri valori di errno vedere man(3) malloc
*/
graph_t * new_graph (unsigned int n, char **lbl);

/** distrugge un grafo deallocando tutta la sua memoria 
 *    \param g puntatore al puntatore al grafo da deallocare -- viene messo a NULL dalla funzione dopo la deallocazione
 */
void free_graph (graph_t** g);

/** stampa il grafo su stdout (formato arbitrario a caratteri)
 *   \param g puntatore al grafo da stampare
 */
void print_graph (graph_t* g);

/** crea una copia completa del grafo (allocando tutta la memoria necessaria)
 *   \param g puntatore al grafo da copiare
 *
 *   \retval p puntatore al nuovo grafo (se tutto e' andato bene)
 *   \retval NULL se si \e verificato un errore
 *   	          errno == EINVAL Parametri errati
 *		  Per altri valori di errno vedere man(3) malloc
 */
graph_t* copy_graph (graph_t* g);

/** aggiunge un arco al grafo (senza ripetizioni, ogni arco deve connettere una diversa coppia sorgente-destinazione)
 *  \param g puntatore al grafo
 *  \param e arco, stringa di formato
 *   LBLSORGENTE:LBLDESTINAZIONE:DISTANZA_IN_KM                                                                           
 *      Esempio: FIRENZE:PRATO:20.4 
 *
 *  Attenzione: le citta possono contenere solo caratteri alfanumerici e lo spazio ' ',
 *  la distanza e' un numero reale,
 *
 *
 *  \retval 0 se l'inserzione e' avvenuta con successo
 *  \retval -1 se si e' verificato un errore (in questo caso il grafo originario non deve essere modificato)
 *	       errno == EINVAL Parametri errati
 *	       errno == EALREADY Se l'arco \e gi\a presente nel grafo
 *		  Per altri valori di errno vedere man(3) malloc
 */
int add_edge (graph_t * g, char* e);


/** verifica l'esistenza di un nodo
 *  \param g puntatore al grafo
 *  \param ss label del nodo
 *
 *  \retval n l'indice nel grafo (0..(n_size -1)) se il nodo esiste
 *  \retval -1 altrimenti
 *  	       errno == EINVAL Parametri errati
*/
int is_node(graph_t* g, char* ss);

/** verifica l'esistenza di un arco
 *   \param g puntatore al grafo
 *   \param n1 etichetta nodo primo estremo
 *   \param n2 etichetta nodo secondo estremo
 *
 *  \retval TRUE se l'arco esiste
 *  \retval FALSE altrimenti
*/
bool_t is_edge(graph_t* g, unsigned int n1, unsigned int n2);

/**  grado di un nodo
 *  \param g puntatore al grafo
 *  \param lbl la label del nodo
 *
 *  \retval n il numero di archi uscenti dal nodo 
 *  \retval -1 se si e' verificato un errore
 *  	       errno == EINVAL se i parametri non sono validi
 */
int degree(graph_t * g, char* lbl);  

/** numero di nodi di un grafo
 *  \param g puntatore al grafo
 *
 *  \retval n il numero di nodi di g 
 *  \retval -1 se si e' verificato un errore
 *             errno == EINVAL se i parametri non sono validi
 *             errno == ENOTSUP se (int)g->size < 0
 */
int n_size(graph_t* g); 

/** numero di archi un grafo
 *  \param g puntatore al grafo
 *
 *  \retval n il numero di archi di g 
 *  \retval -1 se si e' verificato un errore
 *             errno == EINVAL se i parametri non sono validi
 */
int e_size(graph_t* g);

/** carica il grafo da 2 file, il primo contiene le label possibili de nodi, ad esempio:
 *  PISA
 *  FIRENZE
 *  EMPOLI
 *  separate da '\n', il secondo contenente gli archi secondo il formato
 *      LBLSORGENTE:LBLDESTINAZIONE:DISTANZA_IN_KM 
 *  Esempio: FIRENZE:PRATO:20.4
 *  sempre separati da '\n'
 *  
 *  Attenzione: le citta possono contenere solo caratteri alfanumerici e lo spazio ' ', la distanza e' un numero reale, 
 *  ci possono essere linee vuote (cioe' che contengono solo '\n')
 *
 *  \param fdnodes file descriptor del file contenente le label dei nodi
 *  \param fdarcs file descriptor del file contenente gli archi
 *
 *  \retval g puntatore al nuovo grafo se il caricamento e' avvenuto con successo
 *  \retval NULL se si e' verificato un errore (setta errno), es
 *               errno == EINVAL Parametri errati o non validi
 *		 errno == ECANCELED Errore interno (es. malloc fallita)
 *               Per altri valori di errno vedere "graph_t* carica_nodi( FILE* fdnodi, int n_nodi);"
 *
 */
graph_t* load_graph (FILE * fdnodes, FILE * fdarcs);

/** salva il grafo su due file, formato \see load_graph 
 *  \param fdnodes file descriptor del file che dovra' contenere i nodi del grafo
 *  \param fdarcs file descriptor del file che dovra' contenere gli archi del grafo
 *  \param g grafo da scrivere
 *
 *  \retval 0 se la scrittura e' andata a buon fine
 *  \retval -1 se si e' verificato un errore (setta errno), es. 
 *	       errno == EIO se la scrittura e' fallita
 *  	       errno == EINVAL se i parametri sono errati
 */
int save_graph (FILE * fdnodes, FILE * fdarcs, graph_t* g);


/* -= FUNZIONI DEFINITE DA ME PER IL PRIMO FRAMMENTO =- */

/** Cerca l'arco uscente dal nodo di cui conosco la lista di adiacenza e
 *  al nodo di etichetta lbl
 *  \param e La lista di adiacenza del primo estremo dell'arco
 *  \param lbl Secondo estremo dell'arco
 * 
 *  \retval ed Puntatore all'arco cercato
 *  \retval NULL Se l'arco non esiste
 */ 
edge_t* trova_arco(unsigned int lbl, edge_t* e);


/** stampa il grafo su file che deve successivamente essere compilato con il tool dot
 *  \param g puntatore al grafo da stampare
 *  \param filename nome del file su cui stampare il grafo
 *
 *  \retval 0 se tutto ok
 *  \retval -1 se errore 
 *  	       errno == EINVAL Parametri errati
 *	       errno == EIO Errore di scrittura nel file
 */
int print_with_dot(graph_t* g, const char* filename);

/** Valida la stringa città.
 *  Una stringa città è valida se:
 *  (1) E' lunga al più LLABEL (+1 per il fine stringa)
 *  (2) Contiene solo Lettere e/o spazi
 *
 *  \param ss La stringa da validare
 *
 *  \retval 0 Se la stringa \e valida
 *  \retval -1 Altrimenti.
 *             errno == EINVAL -> Parametri errati
 *             errno == EILSEQ -> Stringa non valida
 */
int valida_stringa_citta(char* ss);


/* -= FUNZIONI TERZO FRAMMENTO =- */
/** Crea un nuovo nodo dell'albero degli share.
 *  \param id L'identificatore di chi offre lo share, lungo al massimo LUSERNAME
 *  \param posti Il numero di posti disponibili
 *
 *  \retval nSh Il nuovo share_t creato.
 *  \retval NULL in caso di errore
 *               errno == ENOMEM Se fallisce malloc
 */  
share_t* crea_share(char*id, int posti);

/** Aggiunge un nodo all'abero, in maniera ordinata.
 *  Un tentativo di aggiungere NULL o un elemento gi\a presente
 *  nell'ABR NON modifica l'ABR.
 *  \param shT l'ABR a cui aggiungere il nodo
 *  \param who Il nodo da aggiungere
 *
 *  \retval shT Il nuovo ABR con l'elemento aggiunto
 */
share_t* aggiungi_share(share_t* shT, share_t* who);

/** Trova lo share offerto da id, se esiste.
 *  \param shT L'ABR in cui cercare
 *  \param id L'identificatore da cercare
 *
 *  \retval sh Puntatore allo share_t cercato, se esiste.
 *  \retval NULL Se non esiste (id==NULL non esiste nell'ABR)
 */
share_t* trova_share(share_t* shT, char* id);

/** Trova un qualsiasi share con almeno un posto disponibile
 *  \param shT L'ABR in cui cercare
 *
 *  \retval sh Uno sharae con posti disponibili
 *  \retval NULL Se non ci sono share disponibili
 */
share_t* trova_share_libero(share_t* shT);

/** Rimuove uno share_t dall'ABR, liberando la memoria.
 *  \param shT L'ABR da cui rimuovere
 *  \param id Il nome utente che fornisce lo share da rimuovere
 *
 *  \retval ret L'ABR senza id
 *  \retval NULL In caso di errore (o se ABR era NULL)
 *             errno == EINVAL Parametri non corretti
 *             errno == ENODEV Id non è presente nell'ABR
 */
share_t* rimuovi_share(share_t* shT, char* id);


#endif /* __DGRAPH_H */

