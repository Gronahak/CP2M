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
int nb_shm;
int id_filemessage;


void fin_de_journee(int s){
  printf("L'archiviste de pid [%d] reçoit le signal %d et rentre chez lui.",getpid(),s);
  exit (-1);
}

void mon_sigaction(int signal, void(*f)(int)){
  struct sigaction action;
  action.sa_handler=f;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;
  sigaction(signal,&action,NULL);
}



/* Deux arguments:          */
/*      -numero d'ordre     */
/*      -nb_themes          */

int main (int argc, char *argv[]){

  mon_sigaction(SIGUSR1,fin_de_journee);
  pause();
  int i, clef_filemessage,id_message, clef_journa,id_journa;
  struct tampon message,messageenvoi;
  char* contenu[4],tmp[4];
  FILE * fich_cle;
  char id_lu[50];
  nb_themes=atoi(argv[2]);
  numero_ordre=atoi(argv[1]);
  fprintf(stderr,"test nbtheme %d num ordre %d\n",nb_themes,numero_ordre);
  /* Captage des signaux qui stoppent l'archiviste */



  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("Lancement serveur impossible\n");
    exit(-1);
  }

  /* On recupere les semaphores */
  fgets(id_lu,50,fich_cle);
  //a faire

  fgets(id_lu,50,fich_cle); //2e a incrementer suivant le msgrcv
  
  /* Recuperation de la file de message */

  fgets(id_lu,50,fich_cle);
  clef_filemessage=atoi(id_lu);
  if ((id_filemessage=msgget(clef_filemessage,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation de la file de message chez l'archiviste n°%d.\n",numero_ordre);
    exit(-1);
  }
  /* Récuperation des segments de mémoire partagée de chaque thème */
  i=0;
  while (fgets(id_lu,50,fich_cle)){ // On passe tous les id des shm des themes
    tabclef_shm[i]=atoi(id_lu);
    //fprintf(stdout,"test de connard %d %d %d\n",tabclef_shm[i],atoi(id_lu),i);
    if ((tabid_shm[i]=shmget(tabclef_shm[i],NB_MAX_ARTICLES*4,0))==-1){ /* J'ai mis 50 mais j'aurai très bien pu mettre 51 */
      fprintf(stderr,"Probleme dans la recuperation du segment de mémoire partagée du thème n°%d [archiviste n°%d].\n",i,numero_ordre);
      perror("erreur: ");
      exit(-1);
    }
           i++;
  }    
  nb_shm=i;


  
  fclose(fich_cle);

  
  /* Traitement des messages */
  while(1){
    /* Recuperation du message et traitement */
    /* Si vide > bloque jusqu'à un nouveau message */
    id_message=msgrcv(id_filemessage,&message,sizeof(struct tampon),numero_ordre,MSG_NOERROR);
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



