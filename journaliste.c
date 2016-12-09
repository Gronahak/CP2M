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

  mon_sigaction(SIGUSR1,fin_de_journee);

  // pause();

  int i;

  int  clef_filemessage,id_filemessage,clef_sem_redac_prio,clef_sem_files;
  int id_sem_R_P,id_sem_F;
  int nb_archiviste=atoi(argv[1]);
  char operation=argv[2][0];
  int numjournaliste=getpid(); // Ou un param ?
  int nbarchiv_a_appeler=0;
  // int tmp;
  // struct msqid_ds * info=(struct msqid_ds*)malloc(sizeof(struct msqid_ds));
  struct tampon* message=(struct tampon*)malloc(sizeof(struct tampon));
  //  char * erreur;
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
  //int tab_test[5];
  /*
  for (i=0;i<5;i++)tab_test[i]=0;
  if((semctl(id_sem_F,5,GETALL,tab_test)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
  printf("TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST\n");
  for (i=0;i<5;i++)printf("%d|",tab_test[i]);
  printf("TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST\n");
  */
  
  if((semop(id_sem_F,&P,1))==-1){printf("mutex occupé\n");perror("mutex files occupé");}
  bon_guichet=0;
  file_la_plus_courte=NB_MAX_JOURNALISTES;  
  for (i=1;i<=nb_archiviste;i++){
    if (taille_des_files[i]<file_la_plus_courte){
      bon_guichet=i;
      
      file_la_plus_courte=taille_des_files[i];
    }
    //  printf("\n\nATTENTION LES YEUX %d file la + courte : %d\n\n",taille_des_files[i],file_la_plus_courte);

  }
  printf("  Je suis le journaliste %d et j'ai choisi le guichet %d car il n'y a que %d journalistes dans cette file.\n",getpid(),bon_guichet,taille_des_files[bon_guichet]);
  /* test */
  
  /* test */
  // printf("BONGUICHET NTM NTM NTM BONGUICHET NTM NTM NTMBONGUICHET NTM NTM NTMBONGUICHET NTM NTM NTM%d\n",bon_guichet);
  struct sembuf V2;//{0,1,SEM_UNDO};
  V2.sem_num=bon_guichet;
  V2.sem_op=1;//+taille_des_files[bon_guichet];
  V2.sem_flg=0;
  //  semop(id_sem_F,&V2,0);
    semop(id_sem_F,&V2,1);
    /*
  printf("AAAAAAAAAA\n");
    for (i=0;i<5;i++)tab_test[i]=0;
    if((okok=(semctl(id_sem_F,0,GETVAL,okok))) ==-1){printf("ça déconne2\n");perror("semctl2:");}
   //  for (i=0;i<=nb_archiviste;i++)printf("%d |",tab_test[i])
    
   if((semctl(id_sem_F,1,GETVAL,okok)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
   

   if((semctl(id_sem_F,2,GETVAL,okok)) ==-1){printf("ça déconne2\n");perror("semctl2:");}

   printf("%d",okok);
    */

    //printf("BBBBBBBBB\n");  
  //// rendre mutex
   if((semop(id_sem_F,&V,1))==-1){printf("mutex pas rendu\n");perror("muteximpossible a rendre");}
   // else {printf("\t\tmutex rendu !!\n");}

  // int tab_test[5];
   /*
  for (i=0;i<5;i++)tab_test[i]=0;
  if((semctl(id_sem_F,5,GETALL,tab_test)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
  printf("TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST\n");
  for (i=0;i<5;i++)printf("%d|",tab_test[i]);
  printf("TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST\n");
   */
  /* Et on met son numéro dans nbarchiv_a_appeler                  */
  message->msg_type=bon_guichet; //1>nbarchiv_a_appeler
  message->operation=operation;
  message->num_journaliste=numjournaliste;
  fprintf(stderr,"J'envoie l'operation %c avec le journaliste %d\n",message->operation,message->num_journaliste);
  switch(operation){
  case CONSULTATION:
    fprintf(stderr,"test entree: %d %d\n",atoi(argv[3]),atoi(argv[4]));
    message->theme=atoi(argv[3]);
    message->num_article=atoi(argv[4]);
    break;
  case EFFACEMENT:
    fprintf(stderr,"test entree: %d %d\n",atoi(argv[3]),atoi(argv[4]));
    message->theme=atoi(argv[3]);
    message->num_article=atoi(argv[4]);
    break;
  case PUBLICATION:
    fprintf(stderr,"test entree: %d %s\n",atoi(argv[3]),argv[4]);
    message->theme=atoi(argv[3]);
    strcpy(message->msg_text,argv[4]);
    fprintf(stderr,"entree char : %s ET  %s\n",message->msg_text,argv[4]);

    break;
    }
  
  if (msgsnd(id_filemessage,message,1,IPC_NOWAIT)==-1){
    fprintf(stderr,"Erreur d'envoi d'un message à l'archiviste %d.\n",nbarchiv_a_appeler);
    perror("erreur: ");
  }
    
  /* On attend le message à recevoir */
  //while(1)
  //  msgrcv
  //  break; 
  /* On supprime tous les IPC */
  
  //  msgctl(id_filemessage,IPC_RMID,NULL);

  //ctl des semaphores
  exit(0);
}
