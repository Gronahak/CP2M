/*********************************************************/
/*                                                       */
/*              Programme des Archivistes                */
/*                                                       */
/*********************************************************/

# include "types.h"

int nb_themes;
int numero_ordre;
int tabclef_shm[100];
int tabid_shm[100];
int id_filemessage;

void handler1(int signum){
  int i;
  fprintf(stdout," SIGINT reçu, l'archiviste %d s'arrête et détruit les IPC.\n",numero_ordre);
  /* On supprime les SHM */
  for (i=0; i<nb_themes; i++){
    shmctl(tabid_shm[i],IPC_RMID,NULL);
  }
  /* On supprime la file de message*/
  msgctl(id_filemessage,IPC_RMID,NULL);
  exit(-1);
}


/* Deux arguments:          */
/*      -numero d'ordre     */
/*      -nb_themes          */

int main (int argc, char *argv[]){

  int i, clef_filemessage,id_message;
  struct tampon message;
  
  nb_themes=atoi(argv[2]);
  numero_ordre=atoi(argv[1]);
  
  /* Captage des signaux qui stoppent l'archiviste */

  struct sigaction new;
  new.sa_handler=handler1;
  new.sa_flags=0;
  sigemptyset(&new.sa_mask);
  sigset_t ens,ensvide;
  sigemptyset(&ens);
  sigemptyset(&ensvide);
  sigaddset(&ens,SIGUSR1);
  sigaction(SIGINT,&new,NULL);
  
  /* Lancement des segments de mémoire partagée de chaque thème */
  for (i=0; i<nb_themes; i++){
    tabclef_shm[i]=ftok("archiviste.c",i);
    fprintf(stdout,"test de connard %d\n",tabclef_shm[i]);
    if ((tabid_shm[i]=shmget(tabclef_shm[i],50,IPC_CREAT | IPC_EXCL | 0660))==-1){ /* J'ai mis 50 mais j'aurai très bien pu mettre 51 */
      fprintf(stderr,"Probleme dans la création du segment de mémoire partagée du thème n°%d [archiviste n°%d].\n",i,numero_ordre);
      perror("erreur: ");
      exit(-1);
    }
    
  }
  
  /* Lancement file de message de l'archiviste */
  clef_filemessage=ftok("archiviste.c",numero_ordre);
  if ((id_filemessage=msgget(clef_filemessage,IPC_CREAT | IPC_EXCL | 0660))==-1){
    fprintf(stderr,"Probleme dans la création de la file de message de l'archiviste n°%d.\n",numero_ordre);
    exit(-1);
  }

  /* Traitement des messages */
  while(1){
    /* Recuperation du message et traitement */
    /* Si vide > bloque jusqu'à un nouveau message */
    id_message=msgrcv(id_filemessage,&message,1000,1,MSG_NOERROR);
    // shmat a faire 
    switch(message.operation){
    case 'c': /* consulter l'article */
      break;
    case 'p': /* publier l'article */
      break;
    case 'e': /* supprimer l'article */
      break;
    }
    // de-shmat a faire
  }


  /* A faire en cas d'arret brutal */
  /* On supprime les SHM */
  for (i=0; i<nb_themes; i++){
    shmctl(tabid_shm[i],IPC_RMID,NULL);
  }
  
  /* On supprime la file de message*/
  msgctl(id_filemessage,IPC_RMID,NULL);

  return 0;
}



