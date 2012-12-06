/**
  \file 
  \brief definizione dei prototipi per il calcolo dei cammini minimi si grafo diretto

 \author lso11 */

#ifndef __SHORTESTPATH_H
#define __SHORTESTPATH_H

/** infinito -- valore di distanza per l'output dell'algoritmo di Dijkstra */
#define INFINITY (-1.0)
#define UNDEF ((unsigned int) -1)

/** implementa l'algoritmo di Dijkstra per la ricerca di tutti i cammini minimi a patire  
 *  da un nodo sorgente
 *  \param g grafo pesato, con parametri non negativi
 *  \param source nodo da cui calcolare i cammini minimi
 *  \param pprec puntatore di puntatore: (1) 
 *         se pprec != da NULL viene assegnato a *pprec 
 *         il vettore dei predecessori definito come segue:
 *		   per ogni nodo n1 
 *	               *pprec[n1] = n2
 *	   se n2 e' il nodo che precede n1 nel cammino minimo da source a n1
 *	   (2) se pprec == NULL il vettore non viene assegnato (non e' un errore)
 *
 *  \retval dist puntatore al vettore delle distanze minime calcolate (se tutto e' andato bene )
 *  \retval NULL se si \'e verificato un errore, setta errno
 * 	         errno == EINVAL parametri errati
 *   	         errno == ENOTSUP (int)g->size < 0
 * 	         errno == ENOMEM malloc fallita
 */
double* dijkstra (graph_t* g, unsigned int source, unsigned int** pprec);

/** crea la stringa della rotta da n1 a n2 usando l'array delle precedenze calcolato da dijkstra
    \param g   grafo diretto pesato a costi non negativi
    \param n1  nodo sorgente
    \param n2  nodo destinatario
    \param prec vettore dei predecessori calcolato da Dijkstra

    \retval rotta puntatore alla stringa che descrive la rotta se tutto e' andato bene
    rotta contiene tutte le citta attraversate separate da '$' n1$citta1$....$cittaN$n2
    ad esempio rotta da PISA ad AREZZO PISA$LUCCA$PRATO$FIRENZE$AREZZO
    \retval NULL se la rotta da n1 a n2 non esiste 
    errno == 0 la rotta non esiste
    errno == EINVAL parametri errati
    errno == ENOMEM malloc fallita
 */
char * shpath_to_string(graph_t * g, unsigned int n1, unsigned int n2,  unsigned int * prec);


/* -= FUNZIONI TERZO FRAMMENTO =- */

/** Aggiunge al grafo l'offerta di share (id,posti) sugli archi del cammino minimo
 *  da source a dest
 *  \param g Il grafo che rappresenta la rete stradale
 *  \param source Etichetta del nodo di partenza
 *  \param dest Etichetta del nodo destinazione
 *  \param id Il nome di chi offre lo share
 *  \param posti Numero di posti offerti
 *
 *  \retval 0 Se l'inserimento dell'offerta di share è andata a buon fine
 *  \post Il grafo contiene su tutti gli archi del cammino minimo source..dest l'offerta
 *        di share (id,posti)
 *
 *  \retval -1 In caso di errore -- setta errno
 *             errno == EINVAL Parametri non validi
 *             errno == EBADR Non esiste un cammino da source a dest in strade
 *	       errno == ECANCELED Nel caso l'operazione sia fallita
 *  \post Il grafo non viene modificato
 */
int aggiungi_share_path(graph_t* g, char* source, char* dest, char* id, int posti);


/** Trova un passaggio da source a dest
 *  \param share Il grafo degli share
 *  \param source Etichetta della città di partenza
 *  \param dest Etichetta della città di arrivo
 *  \param richiedente Chi richiede lo share
 *
 *  \retval ret Array contenente lo/gli share che mi consentono di arrivare da source a dest
 *              Ad esempio se io richiedo un passaggio da Pisa a Roma e gentile_utente{1,2}
 *              mi consentono di arrivarci l'array ret sarà fatto così:
 *              ret[0] = "gentile_utente1$richiedente$Pisa$Firenze"
 *              ret[1] = "gentile_utente2$richiedente$Firenze$Grosseto$...$Roma"
 *              ret[2] = NULL
 *  \retval NULL Nel caso non siano disponibili passaggi da source a dest oppure in caso di errore.
 *	         errno == 0 Non è possibile arrivare da source a dest
 *               errno == EINVAL Parametri non validi
 *
 *  \post Tutti gli share_t che contribuiscono al passaggio devono aver decrementato i posti disponibili.
 *  \post Tutti gli share_t che hanno posti=0 vengono eliminati
 *  \post Tutti gli archi che hanno share_tree vuoto vengono eliminati
 */
char** trova_un_passaggio(graph_t* share, char* source, char* dest, char* richiedente);


#endif /* __SHORTESTPATH_H */
