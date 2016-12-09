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
extern int nombre_redacteurs;
extern int nombre_lecteurs;

void fin_de_journee(int s){
  printf("L'archiviste de pid [%d] reçoit le signal %d et rentre chez lui.\n",getpid(),s);
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

void modification_num_sem(int num,struct sembuf *action){
  action->sem_num=num;
}

int main (int argc, char *argv[]){
  sigset_t sig;
  sigfillset(&sig);
  sigdelset(&sig,SIGUSR1);
  sigprocmask(SIG_BLOCK,&sig,NULL);
  
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
  /*
    ushort tab[5]={0};
    for (i=0;i<5;i++)tab[i]=0;
  */
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
  if(fgets(id_lu,50,fich_cle)==NULL)
    printf("Erreur lecture du fichier\n");
  clef_sem_redac_prio=atoi(id_lu);
  if ((id_sem_R_P=semget(clef_sem_redac_prio,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore propre à l'execution chez l'archiviste n°%d.\n",numero_ordre);
    exit(-1);
  }
  printf("\x1b[32m\n");


  // printf("\n");
  //  int valeurhihi=6;
  
  
  
  /*    2) ensemble de semaphores des files d'attentes archivistes*/
  if(fgets(id_lu,50,fich_cle)==NULL)
    printf("Erreur lecture du fichier\n");
  clef_sem_files=atoi(id_lu);
  if ((id_sem_F=semget(clef_sem_files,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore de gestion des files chez l'archiviste n°%d.\n",numero_ordre);
    exit(-1);}
 
  printf("\n");

  taille_de_la_file=semctl(id_sem_F,numero_ordre,GETVAL);

  
  // printf("[[[[[[[[[[[[[[[%d |",taille_de_la_file);
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
    sleep(rand()%6);
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
      //// Prendre le sémaphore (avant) qui est le 2eme:
      modification_num_sem(2,&P);
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (AVANT)occupé\n");perror("mutex (AVANT) occupé");}
      //// Prendre le sémaphore (lecture) qui est le 1er:
      modification_num_sem(1,&P);      
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (LECTURE)occupé\n");perror("mutex (LECTURE) occupé");}
      //// Prendre le sémaphore (nombre) qui est le 3eme:
      modification_num_sem(3,&P);      
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (NOMBRE)occupé\n");perror("mutex (NOMBRE) occupé");}
      nombre_lecteurs++;
      if (nombre_lecteurs==1){//Premier lecteur 
	//// Prendre le semaphore du theme concerné
	modification_num_sem(4+message->theme,&P);      
	if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (theme n)occupé\n");perror("mutex (theme n) occupé");}
      }
      //// Rendre le sémaphore (nombre) qui est le 3eme:
      modification_num_sem(3,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (NOMBRE) pas rendu\n");perror("mutex Nombre impossible  a rendre");}

      //// Rendre le sémaphore (lecture) qui est le 1er:
      modification_num_sem(1,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (LECTURE) pas rendu\n");perror("mutex Lecture impossible  a rendre");}

      //// Rendre le sémaphore (avant) qui est le 2eme:
      modification_num_sem(2,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (AVANT) pas rendu\n");perror("mutex Avant impossible  a rendre");}

      //// Lecture du fichier
      message_envoi->operation='c';
      if (message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	printf("\t[Archiviste %d] Numéro d'article non existant (consultation)\n",numero_ordre);
	strcpy(message_envoi->msg_text,"ERRN");
      }
      else{
	for (i=0;i<4;i++)
	  message_envoi->msg_text[i]=contenu[4*message->num_article+i];
	printf("\t[Archiviste %d] Consultation de l'article %d (theme %d)\n",numero_ordre,message->num_article,message->theme);
      }
      //// Prendre le sémaphore (nombre) qui est le 3eme:
      modification_num_sem(3,&P);      
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (NOMBRE)occupé\n");perror("mutex (NOMBRE) occupé");}
      nombre_lecteurs--;

      if (nombre_lecteurs==0){//Dernier lecteur 
	//// Rendre le sémaphore du theme concerné :
	modification_num_sem(4+message->theme,&V);      
	if((semop(id_sem_R_P,&V,1))==-1){printf("mutex theme n pas rendu\n");perror("mutex theme n impossible  a rendre");}
      }

      //// Rendre le sémaphore (nombre) qui est le 3eme:
      modification_num_sem(3,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (NOMBRE) pas rendu\n");perror("mutex Nombre impossible  a rendre");}

      
      break;
      
    case PUBLICATION: /* publier l'article */

      //// Prendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&P);
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (redacteurs)occupé\n");perror("mutex (redacteurs) occupé");}
      nombre_redacteurs++;

      if (nombre_redacteurs==1){//Premier redacteur 

	//// Prendre le sémaphore (lecture) qui est le 1er:

	modification_num_sem(1,&P);      
	printf("RECEPTION______________%d_____\n",P.sem_num);

	//	if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (LECTURE)occupé\n");perror("mutex (LECTURE) occupé");}
	printf("RECEPTION___________________\n");

      }

      //// Rendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (redacteurs) pas rendu\n");perror("mutex redacteurs impossible  a rendre");}

      //// Prendre le semaphore du theme concerné
      modification_num_sem(4+message->theme,&P);      
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (theme n)occupé\n");perror("mutex (theme n) occupé");}
	
      //Ecriture

      
      message_envoi->operation='p';
      while (contenu[indice]!='-' && indice<4*NB_MAX_ARTICLES){
	indice+=4;
	numarti++;
      }
      if(indice==4*NB_MAX_ARTICLES){
	strcpy(message_envoi->msg_text,"ERMA");
	printf("\t[Archiviste %d] Nombre maximum d'aricles atteint pour le theme %d (publication).\n",numero_ordre,message->theme);
       
      }
      else {
	for (i=0;i<4;i++)
	  contenu[4*numarti+i]=message->msg_text[i];
	message_envoi->num_article=numarti;
	printf("\t[Archiviste %d] Article %d [%s] (theme %d) publié.\n",numero_ordre,numarti,message->msg_text,message->theme);
      }
      // Fin Ecriture

 
      //// Rendre le sémaphore du theme concerné :
      modification_num_sem(4+message->theme,&V);      
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex theme n pas rendu\n");perror("mutex theme n impossible  a rendre");}
      
      //// Prendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&P);
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (redacteurs)occupé\n");perror("mutex (redacteurs) occupé");}
      nombre_redacteurs--;

      if(nombre_redacteurs==0){// Dernier redacteur
	
	//// Rendre le sémaphore (lecture) qui est le 1er:
	modification_num_sem(1,&V);
	if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (LECTURE) pas rendu\n");perror("mutex Lecture impossible  a rendre");}

      }
      //// Rendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (redacteurs) pas rendu\n");perror("mutex redacteurs impossible  a rendre");}



      
      break;
    case EFFACEMENT: /* supprimer l'article */
      //// Prendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&P);
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (redacteurs)occupé\n");perror("mutex (redacteurs) occupé");}
      nombre_redacteurs++;
      if (nombre_redacteurs==1){//Premier redacteur 
	//// Prendre le sémaphore (lecture) qui est le 1er:
	modification_num_sem(1,&P);      
	if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (LECTURE)occupé\n");perror("mutex (LECTURE) occupé");}
      }
      //// Rendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (redacteurs) pas rendu\n");perror("mutex redacteurs impossible  a rendre");}

      //// Prendre le semaphore du theme concerné
      modification_num_sem(4+message->theme,&P);      
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (theme n)occupé\n");perror("mutex (theme n) occupé");}
	
      //Ecriture
      message_envoi->operation='e';
      if(message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	printf("\t[Archiviste %d] Effacement de l'article %d (theme %d) impossible (non existant).\n",numero_ordre,message->num_article,message->theme);
	strcpy(message_envoi->msg_text,"ERNE");
      }
      else{
	for (i=0;i<4;i++)
	  contenu[4*message->num_article+i]='-';
	fprintf(stdout,"\t[Archiviste %d] Article %d (theme %d) supprimé.\n",numero_ordre,message->num_article,message->theme);
      }
      //Fin ecriture

      
      //// Rendre le sémaphore du theme concerné :
      modification_num_sem(4+message->theme,&V);      
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex theme n pas rendu\n");perror("mutex theme n impossible  a rendre");}
      
      //// Prendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&P);
      if((semop(id_sem_R_P,&P,1))==-1){printf("mutex (redacteurs)occupé\n");perror("mutex (redacteurs) occupé");}
      nombre_redacteurs--;

      if(nombre_redacteurs==0){// Dernier redacteur
	
	//// Rendre le sémaphore (lecture) qui est le 1er:
	modification_num_sem(1,&V);
	if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (LECTURE) pas rendu\n");perror("mutex Lecture impossible  a rendre");}

      }
      //// Rendre le sémaphore (redacteurs) qui est le 0eme:
      modification_num_sem(0,&V);
      if((semop(id_sem_R_P,&V,1))==-1){printf("mutex (redacteurs) pas rendu\n");perror("mutex redacteurs impossible  a rendre");}


      break;
      
    default: break;
    }
    /* On previent le journaliste de l'action */
    message_envoi->msg_type=message->num_journaliste;
    if (msgsnd(id_filemessage,message_envoi,sizeof(struct tampon),0)==-1)

      shmdt(&contenu);
    printf("\n");

    
    taille_de_la_file=semctl(id_sem_F,numero_ordre,GETVAL);

    //// prendre mutex file
    modification_num_sem(0,&P);

    if((semop(id_sem_F,&P,1))==-1){printf("mutex occupé\n");perror("mutex files occupé");}

    struct sembuf V2;
    V2.sem_num=numero_ordre;
    V2.sem_op=-1;
    V2.sem_flg=0;

    printf("AVANT OPAVANT OPAVANT OPAVANT OPAVANT OPAVANT OPAVANT OPAVANT OP\n");
    printf("_______________________\n");
    for (i=1;i<numero_ordre;i++)printf("\t");
    printf("|%d|",taille_de_la_file);
    semop(id_sem_F,&V2,1);

  
    taille_de_la_file=semctl(id_sem_F,numero_ordre,GETVAL);

    //  printf("APRES OPAPRES OPAPRES OPAPRES OPAPRES OPAPRES OPAPRES OPAPRES OP\n");
    printf("|%d|\n",taille_de_la_file);
    printf("_______________________\n");
    //// rendre mutex file

    modification_num_sem(0,&V);
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



