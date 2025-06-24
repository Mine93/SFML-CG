Non ho iniziato a tenere traccia dei progressi del progetto fino a questo punto, tutta la parte di gestione della finestra
e l'event handling l'ho copiato da breakout, lo State contiene il player, 4 variabili booleane che vengono usate per comunicare
al player in che direzione muoversi, come abbiamo visto a lezione, ho implementato le funzioni draw e update su State e Player.
Il player riceve in input nella funzione update un vector2f contenente la direzione ottenuta dalle 4 booleane dello State, la
sprite sheet del player è strutturata in 8 righe che rappresentano le 4 direzioni in cui il personaggio può rivolgersi con
animazione idle e di camminata, a seconda della direzione indicata dal vettore in input viene scelta quale riga del sheet usare
le prime 4 righe per quando il personaggio è fermo e le ultime 4 quando è in movimento, tenendo traccia del tempo passato,
aggiorniamo quale steo dell'animazione usare tramite la funzione update_animation.
La meccanica base del gioco è il "dash", funziona tramite After_Image "immagine postuma" non è altro che una struct che funge
da punto di salvataggio della posizione e della sprite attuale del giocatore, premendo shift e tenendolo premuto, possiamo dare
inizio ad un dash grazie alla funzione start_dash, qui una nuova After_Image viene creata e gli viene assegnata la posizione e
sprite attuale del giocatore, in aggiunta viene disegnata una linea che collega il player alla after image, tramite una
RectangleShape di lunghezza pari alla distanza tra il player e la after image, la funzione dist l'ho presa da breakout,
cambiando l'angolo del rettangolo tramite la funzione angle (che ho trovato su internet), possiamo creare l'effetto
della linea che collega i due, appena rilasciamo shift la posizione del player viene impostata a quella della after image e
quest'ultima viene cancellata.