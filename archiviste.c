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

  int i, clef_filemessage,clef_sem_redac_prio,clef_sem_files;
  int id_sem_R_P,id_sem_F;
  struct tampon* message=(struct tampon*)malloc(sizeof(struct tampon));
  struct tampon* message_envoi=(struct tampon*)malloc(sizeof(struct tampon));
  char* contenu;
  FILE * fich_cle;
  char id_lu[50];
  struct sembuf P={0,-1,SEM_UNDO};
  struct sembuf V={0,+1,SEM_UNDO};    
  


  int  taille_de_la_file=0;
  /*
  int *taille_des_files;
  taille_des_files=(int *)malloc((1+nb_archiviste)*sizeof(int));
  if (taille_des_files==NULL){ printf("Echec du malloc plus de memoire.\n"); exit(-1);}
  */
  ushort tab[5]={0};
  for (i=0;i<5;i++)tab[i]=0;
  
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
  /*    1) ensemble de semaphores propre à l'execution */
  fgets(id_lu,50,fich_cle);
  clef_sem_redac_prio=atoi(id_lu);
  if ((id_sem_R_P=semget(clef_sem_redac_prio,0,0))==-1){
     fprintf(stderr,"Probleme dans la recuperation du sémaphore propre à l'execution chez l'archiviste n°%d.\n",numero_ordre);
    exit(-1);
 
  }
     printf("\x1b[32m\n");

     printf("val semaphore arch %d: \n",numero_ordre);
  if((semctl(id_sem_R_P,5,GETALL,tab)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
  for (i=0;i<5;i++)printf("%d |",tab[i]);

  // printf("\n");
  //  int valeurhihi=6;
  
  
  
  /*    2) ensemble de semaphores des files d'attentes archivistes*/
  fgets(id_lu,50,fich_cle);
  clef_sem_files=atoi(id_lu);
  if ((id_sem_F=semget(clef_sem_files,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore de gestion des files chez l'archiviste n°%d.\n",numero_ordre);
    exit(-1);}
 
  printf("\n");

    taille_de_la_file=semctl(id_sem_F,numero_ordre,GETVAL);

  
  printf("[[[[[[[[[[[[[[[%d |",taille_de_la_file);
  printf("\n");

  printf("\033[0m\n");

 

  
  /* Recuperation de la file de message */

  if (fgets(id_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
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
  printf("\n");
  /* Traitement des messages */
  while(1){
    int indice=0;
    int numarti=0;
    /* Recuperation du message et traitement */
    /* Si vide > bloque jusqu'à un nouveau message */
    // fprintf(stderr,"MESSAGE a recevoir\n");
    msgrcv(id_filemessage,message,sizeof(struct tampon),numero_ordre,MSG_NOERROR);
    contenu=shmat(tabid_shm[message->theme],0,0);
    // fprintf(stderr,"Testinfos: %d OU %s et operation %c theme %d\n",message->num_journaliste,message->msg_text,message->operation,message->theme);

    /* /!\ Ouvrir la file de message du jouraliste pour lui envoyer un message */ 
    switch(message->operation){
    case CONSULTATION: /* consulter l'article */
      message_envoi->operation='c';
      if (message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	printf("\t[Archiviste %d] Numéro d'article non existant (consultation)\n",numero_ordre);
	strcpy(message_envoi->msg_text,"ERRN");
	break;
      }
      for (i=0;i<4;i++)
	message_envoi->msg_text[i]=contenu[4*message->num_article+i];
      printf("\t[Archiviste %d] Consultation de l'article %d (theme %d)\n",numero_ordre,message->num_article,message->theme);
      break;
    case PUBLICATION: /* publier l'article */
      /* verif semaphore theme */
      message_envoi->operation='p';
      while (contenu[indice]!='-' && indice<4*NB_MAX_ARTICLES){
	indice+=4;
	numarti++;
      }
      if(indice==4*NB_MAX_ARTICLES){
	strcpy(message_envoi->msg_text,"ERMA");
	printf("\t[Archiviste %d] Nombre maximum d'aricles atteint pour le theme %d (publication).\n",numero_ordre,message->theme);
	break;
      }
      for (i=0;i<4;i++)
	contenu[4*numarti+i]=message->msg_text[i];
      message_envoi->num_article=numarti;
      printf("\t[Archiviste %d] Article %d [%s] (theme %d) publié.\n",numero_ordre,numarti,message->msg_text,message->theme);
	    
      break;
    case EFFACEMENT: /* supprimer l'article */
      /* verif semaphore theme */
      message_envoi->operation='e';
      if(message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	printf("\t[Archiviste %d] Effacement de l'article %d (theme %d) impossible (non existant).\n",numero_ordre,message->num_article,message->theme);
	strcpy(message_envoi->msg_text,"ERNE");
	break;
      }
      for (i=0;i<4;i++)
	contenu[4*message->num_article+i]='-';
      fprintf(stdout,"\t[Archiviste %d] Article %d (theme %d) supprimé.\n",numero_ordre,message->num_article,message->theme);
      break;
    default: break;
    }
    /* On previent le journaliste de l'action */
    message_envoi->msg_type=message->num_journaliste;
    if (msgsnd(id_filemessage,message_envoi,sizeof(struct tampon),0)==-1)

    shmdt(&tabid_shm[message->theme]);
    printf("\n");

    
    taille_de_la_file=semctl(id_sem_F,numero_ordre,GETVAL);

      //// prendre mutex

  if((semop(id_sem_F,&P,1))==-1){printf("mutex occupé\n");perror("mutex files occupé");}


  //// rendre mutex
   if((semop(id_sem_F,&V,1))==-1){printf("mutex pas rendu\n");perror("muteximpossible a rendre");}
  
  
  printf("[[[[[[[[[[[[[[[%d |",taille_de_la_file);
    /* ON réinitialise*/
    strcpy(message_envoi->msg_text,"NADA");
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



