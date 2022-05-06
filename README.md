# TicTacToe
Progetto LSO 2020/2021 \
Realizzare un sistema client-server che consenta ai client di giocare a tris contro altri giocatori. \
Descrizione dettagliata\
Quando un nuovo client si collega al server, viene messo in attesa finché non è disponibile un altro client periniziare una partita.\
A quel punto, i due client iniziano la partita, comunicando le loro mosse al server.Il server deve essere in grado di gestire un numero arbitrario di partite simultanee (ogni partita coinvolgedue client e ogni client può essere coinvolto in una sola partita alla volta).\
Al termine di una partita, i dueclient coinvolti tornano disponibili e possono venire accoppiati con altri client disponibili, se ce ne sono in quelmomento, per poi cominciare una nuova partita.\
I client possono anche visualizzare la classifica attuale dei giocatori (identificati da un nicknamescelto dal giocatore)\
