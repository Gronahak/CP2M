/*********************************************************/
/*                                                       */
/*              Programme des Journaliste                */
/*                                                       */
/*********************************************************/

# include "types.h"


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
/* Quatres arguments:                               */
/*      -nombre d'archivistes                       */
/*      -operation (c,p,e) +2autres/operation       */
/* c: theme && numero article                       */
/* p: theme && texte article                        */
/* e: theme && numero article                       */

int main (int argc, char *argv[]){
  printf("TEST %s & %s & %s\n",argv[1],argv[2],argv[3]);
  mon_sigaction(SIGUSR1,fin_de_journee);

  // pause();

  int i;

  int  clef_filemessage,id_filemessage,clef_sem_redac_prio,clef_sem_files;
  int id_sem_R_P,id_sem_F;
  int nb_archiviste=atoi(argv[1]);

  char operation=argv[2][0];
  int numjournaliste=getpid(); // Ou un param ?
  int nbarchiv_a_appeler=0;
  int id_message;
  // int tmp;
  // struct msqid_ds * info=(struct msqid_ds*)malloc(sizeof(struct msqid_ds));
  struct tampon* message=(struct tampon*)malloc(sizeof(struct tampon));
  struct tampon* message_recu=(struct tampon*)malloc(sizeof(struct tampon));
  FILE *fich_cle;
  char id_lu[100];
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("Fichier inexistant\n");
    exit(-1);
  }

  struct sembuf P={0,-1,SEM_UNDO};
  struct sembuf V={0,+1,SEM_UNDO};    
  int *taille_des_files;
  int file_la_plus_courte=NB_MAX_JOURNALISTES;
  int bon_guichet;
  taille_des_files=(int *)malloc((1+nb_archiviste)*sizeof(int));
  if (taille_des_files==NULL){ printf("Echec du malloc plus de memoire.\n"); exit(-1);}
  for (i=0;i<nb_archiviste;i++)taille_des_files[i]=0;
  ushort tab[5]={0};
  for (i=0;i<5;i++)tab[i]=0;
  
  /* On recupere les semaphores */
  /*    1) ensemble de semaphores propre à l'execution */
  fgets(id_lu,50,fich_cle); 
  clef_sem_redac_prio=atoi(id_lu);
  if ((id_sem_R_P=semget(clef_sem_redac_prio,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore propre à l'execution chez le journaliste n°%d.\n",getpid());
    exit(-1);
 
  }

    if((semctl(id_sem_R_P,5,GETALL,tab)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
    // for (i=0;i<5;i++)printf("%d |",tab[i]);



    //  printf("\n");
  /* On recupere l'ensemble 2 de semaphore */
  
 
  /*    2) ensemble de semaphores des files d'attentes archivistes*/
  fgets(id_lu,50,fich_cle);
  clef_sem_files=atoi(id_lu);
  if ((id_sem_F=semget(clef_sem_files,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore de gestion des files chez le journaliste n°%d.\n",getpid());
    exit(-1);
  }

  /*
    if((semctl(id_sem_F,5,GETALL,taille_des_files)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
    printf("Je récupere la taille des files:\n");
    for (i=0;i<=nb_archiviste;i++)printf("%d |",taille_des_files[i]);
  */

  for (i=0;i<=nb_archiviste;i++){
    taille_des_files[i]=semctl(id_sem_F,i,GETVAL);

  }

   printf("\n");


   //     printf("DDDDDDDDDDDD\n");
     // for (i=0;i<5;i++)tab_test[i]=0;
   // int okok=0;
    //   if((semctl(id_sem_F,0,GETVAL,okok)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
   //  for (i=0;i<=nb_archiviste;i++)printf("%d |",tab_test[i]);
    /*
    okok=(semctl(id_sem_F,0,GETVAL));
   printf("%d\n",okok);

 okok=(semctl(id_sem_F,1,GETVAL));
   printf("%d\n",okok);
    okok=(semctl(id_sem_F,2,GETVAL));

   printf("%d\n",okok);


   printf("CCCCCCCC\n"); */


  /*Lancement file de message */
      

  fgets(id_lu,50,fich_cle); // archiviste

  clef_filemessage=atoi(id_lu);

  if ((id_filemessage=msgget(clef_filemessage,0))==-1){ 
    fprintf(stderr,"Probleme dans la recuperation de la file de message.journ %d\n",getpid());

    perror("erreur: ");
    exit(-1);
  }
  
  fclose(fich_cle);
  /* On cherche quel archiviste a le moins de message dans sa file avec les semaphores */
  //// prendre mutex

  
  if((semop(id_sem_F,&P,1))==-1){printf("mutex occupé\n");perror("mutex files occupé");}
  bon_guichet=0;
  file_la_plus_courte=NB_MAX_JOURNALISTES;  
  for (i=1;i<=nb_archiviste;i++){
    if (taille_des_files[i]<file_la_plus_courte){
      bon_guichet=i;
      
      file_la_plus_courte=taille_des_files[i];
    }

  }
  printf("  Je suis le journaliste %d et j'ai choisi le guichet %d car il n'y a que %d journalistes dans cette file.\n",getpid(),bon_guichet,taille_des_files[bon_guichet]);
 
  struct sembuf V2;
  V2.sem_num=bon_guichet;
  V2.sem_op=1;
  V2.sem_flg=0;
    semop(id_sem_F,&V2,1);
  //// rendre mutex
   if((semop(id_sem_F,&V,1))==-1){printf("mutex pas rendu\n");perror("muteximpossible a rendre");}
  message->msg_type=bon_guichet; 

  message->operation=operation;
  message->num_journaliste=numjournaliste;
  fprintf(stderr,"J'envoie l'operation %c avec le journaliste %d\n",message->operation,message->num_journaliste);
  switch(operation){
  case CONSULTATION:
    //fprintf(stderr,"test entree: %d %d\n",atoi(argv[3]),atoi(argv[4]));
     message->theme=atoi(argv[3]);
    message->num_article=atoi(argv[4]);
    break;
  case EFFACEMENT:
    // fprintf(stderr,"test entree: %d %d\n",atoi(argv[3]),atoi(argv[4]));
    message->theme=atoi(argv[3]);
    message->num_article=atoi(argv[4]);
    break;
  case PUBLICATION:
    //fprintf(stderr,"test entree: %d %s\n",atoi(argv[3]),argv[4]);
    message->theme=atoi(argv[3]);
    strcpy(message->msg_text,argv[4]);
    fprintf(stderr,"entree char : %s ET  %s\n",message->msg_text,argv[4]);

    break;
    }
  printf("[Journaliste %d] demande à l'archiviste %d de traiter sa requête\n",message->num_journaliste,1); //faut pas laisser 1
  if (msgsnd(id_filemessage,message,sizeof(struct tampon),0)==-1){
    fprintf(stderr,"Erreur d'envoi d'un message à l'archiviste %d.\n",nbarchiv_a_appeler);
    perror("erreur: ");
  }
    
  /* On attend le message à recevoir */

  while(1){
    if ((id_message=msgrcv(id_filemessage,message_recu,(sizeof(struct tampon)),getpid(),MSG_NOERROR))>0){
      /* Affichage du résultat de la requête */
      //printf("Je reçois %s.\n",message_recu->msg_text);
      
      switch (message_recu->operation){
      case 'c':
	if (strcmp(message_recu->msg_text,"ERRN")==0)
	  printf("[Journaliste %d] La consultation a échouée, le numéro d'article %d (theme %d) n'existe pas.\n",message->num_journaliste,message->num_article,message->theme);
	else printf("[Journaliste %d] Consultation de l'article %d [%s] (theme %d) réussie.\n",message->num_journaliste,message->num_article,message_recu->msg_text,message->theme);
	break;
      case 'p':
	if (strcmp(message_recu->msg_text,"ERMA")==0)
	  printf("[Journaliste %d] La publication a échouée, le nombre d'articles maximum pour le theme %d est atteint.\n",message->num_journaliste,message->theme);
	else printf("[Journaliste %d] Publication de l'article %d [%s] (theme %d) réussie.\n",message->num_journaliste,message_recu->num_article,message->msg_text,message->theme);
	break;
      case 'e':
	if (strcmp(message_recu->msg_text,"ERNE")==0)
	  printf("[Journaliste %d] L'effacement a échoué, l'article %d (theme %d) n'existe pas.\n",message->num_journaliste,message->num_article,message->theme);
	else printf("[Journaliste %d] Effacement de l'article %d (theme %d) réussi.\n",message->num_journaliste,message->num_article,message->theme);
	}
      break; /* On casse la boucle while */
    }
  }
  
  exit(0);
}
