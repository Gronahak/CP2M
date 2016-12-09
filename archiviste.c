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

void handler1(int signum){
  int i;
  /*
  for(i=0;i<nb_shm;i++){
    shmctl(tabclef_shm[i],IPC_RMID,NULL);
    printf("EH\n");
  }
  */
  exit(-1);
}


/* Deux arguments:          */
/*      -numero d'ordre     */
/*      -nb_themes          */

int main (int argc, char *argv[]){

  int i, clef_filemessage,id_message,id_journa;
  struct tampon* message=(struct tampon*)malloc(sizeof(struct tampon)),messageenvoi;
  struct sigaction new;
  sigset_t ens,ensvide;
  char* contenu[4],tmp[4];
  FILE * fich_cle;
  char id_lu[50];
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
        fprintf(stderr,"MESSAGE a recevoir\n");
	id_message=msgrcv(id_filemessage,message,sizeof(struct tampon),numero_ordre,MSG_NOERROR);
    fprintf(stderr,"MESSAGE RECU chez archiv %d\n",numero_ordre);
    *contenu=shmat(tabid_shm[message->theme],NULL,0);
    fprintf(stderr,"MESSAGE traité ????? ET:%d\n",message->num_journaliste);
    //strcpy(tmp,message.msg_text);

    /* /!\ Ouvrir la file de message du jouraliste pour lui envoyer un message */ 
    switch(message->operation){
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
      //*contenu=strtok(*contenu,tmp);//NON
      fprintf(stdout,"Article supprimé BOYAH\n");
      break;
    default: break;
    }
    /* On previent le journaliste de l'action */
    //  if (msgsnd(clef_journa,&messageenvoi,10,IPC_NOWAIT)==-1)
    //exit(-1);
   
    shmdt(&id_message);
    fprintf(stderr,"MESSAGE traité ?\n");
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



