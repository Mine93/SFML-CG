Eseguito refactoring della maggior parte del codice, funzionalmente identico al precedente tranne per il fatto che i Ghost adesso spawnano
fuori dallo schermo in modo pseudo-randomico e il bug della lista è stato risolto.
La logica dietro al refactoring sta al fatto che praticmante tutte le struct necessarie per il funzionamente del gioco
hanno bisogno delle funzioni update e/o draw per questo ho creato una struct astratta Updatable che contiene queste due,
dalla quale le altre struct possono ereditare.