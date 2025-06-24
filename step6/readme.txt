Qui ho aggiunto la la collisione tra la linea e il Ghost, completando il sistema di combattimento del gioco.
Chiamando la funzione edgeIntersetcs, possiamo verificare la linea si interseca con un qualsiasi lato della sprite
del Ghost, in quel caso abbiamo colpito il ghost.
In più, adesso, il dash riporta il giocatore nella posizione delle After_Image solo se si riesce a colpire un Ghost
con la linea, se si manca si resta nella posizione attuale.
Aggiunta anche la vita, rappresentata da 3 cuori in alto a sinistra, ogni colpo subito da parte di un Ghost se ne
perde uno, persi 3 cuori si perde.