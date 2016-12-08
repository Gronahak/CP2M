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
  fprintf(stdout," \n Arret brutal:\n SIGINT reçu, l'archiviste %d s'arrête et détruit les IPC.\n",numero_ordre);
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

  int i, clef_filemessage,id_message, clef_journa,id_journa;
  struct tampon message,messageenvoi;
  struct sigaction new;
  sigset_t ens,ensvide;
  char* contenu[4],tmp[4];
  char clef_filemess[50];
  FILE * fich_cle;
  char id_lu[100];
  nb_themes=atoi(argv[2]);
  numero_ordre=atoi(argv[1]);
  fprintf(stderr,"test nbtheme %d num ordre %d\n",nb_themes,numero_ordre);
  /* Captage des signaux qui stoppent l'archiviste */

  new.sa_handler=handler1;
  new.sa_flags=0;
  sigemptyset(&new.sa_mask);
  sigemptyset(&ens);
  sigemptyset(&ensvide);
  sigaddset(&ens,SIGUSR1);
  sigaction(SIGINT,&new,NULL);

  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("Lancement serveur impossible\n");
    exit(-1);
  }

  /* On recupere les semaphores */
  fgets(id_lu,50,fich_cle);
  //a faire
        
  /* Lancement des segments de mémoire partagée de chaque thème */
  fgets(id_lu,50,fich_cle); // On passe tous les id des shm des themes
  
  for (i=0; i<nb_themes; i++){
    fgets(id_lu,50,fich_cle); // On passe tous les id des shm des themes
    tabclef_shm[i]=atoi(id_lu);
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

  sprintf(clef_filemess,"%d",clef_filemessage);
  fputs(clef_filemess,fich_cle);
  fclose(fich_cle);
  
  /* Traitement des messages */
  while(1){
    /* Recuperation du message et traitement */
    /* Si vide > bloque jusqu'à un nouveau message */
    id_message=msgrcv(id_filemessage,&message,sizeof(struct tampon),1,MSG_NOERROR);
    *contenu=shmat(tabid_shm[message.theme],NULL,0);
    strcpy(tmp,message.msg_text);
    clef_journa=ftok("journaliste.c",message.num_journaliste);
    if ((id_journa=msgget(clef_filemessage,IPC_CREAT | IPC_EXCL | 0660))==-1){
      fprintf(stderr,"Probleme dans la recuperation de la file de message du journaliste n°%d.\n",message.num_journaliste);
      exit(-1);
    }

    /* /!\ Ouvrir la file de message du jouraliste pour lui envoyer un message */ 
    switch(message.operation){
    case CONSULTATION: /* consulter l'article */
      /* envoie un message avec le contenu */
      break;
    case PUBLICATION: /* publier l'article */
      /* verif semaphore theme */
      strcat(*contenu,tmp);
      fprintf(stdout,"Article publié MAGUEULE\n");
      break;
    case EFFACEMENT: /* supprimer l'article */
      /* verif semaphore theme */
      *contenu=strtok(*contenu,tmp);//NON
      fprintf(stdout,"Article supprimé BOYAH\n");
      break;
    }
    /* On previent le journaliste de l'action */
    if (msgsnd(clef_journa,&messageenvoi,10,IPC_NOWAIT)==-1)
       exit(-1);
   
    msgctl(id_journa,IPC_RMID,NULL);
    
    shmdt(&id_message);
  }

  /* On supprime les sémaphores */
  
  /* On supprime les SHM */
  for (i=0; i<nb_themes; i++){
    shmctl(tabid_shm[i],IPC_RMID,NULL);
  }
  
  /* On supprime la file de message*/
  msgctl(id_filemessage,IPC_RMID,NULL);

  return 0;
}



