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
  // int nb_archiviste=atoi(argv[1]);
  char operation=argv[2][0];
  int numjournaliste=getpid(); // Ou un param ?
  int nbarchiv_a_appeler=0;
  // int tmp;
  // struct msqid_ds * info=(struct msqid_ds*)malloc(sizeof(struct msqid_ds));
  struct tampon message;
  //  char * erreur;
  FILE *fich_cle;
  char id_lu[100];
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("Fichier inexistant\n");
    exit(-1);
  }

  
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
  for (i=0;i<5;i++)printf("%d |",tab[i]);

  printf("\n");
  /* On recupere l'ensemble 2 de semaphore */
  
 
  /*    2) ensemble de semaphores des files d'attentes archivistes*/
  fgets(id_lu,50,fich_cle);
  clef_sem_files=atoi(id_lu);
  if ((id_sem_F=semget(clef_sem_files,0,0))==-1){
    fprintf(stderr,"Probleme dans la recuperation du sémaphore de gestion des files chez le journaliste n°%d.\n",getpid());
    exit(-1);
  }
    if((semctl(id_sem_F,5,GETALL,tab)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
  for (i=0;i<5;i++)printf("%d |",tab[i]);

  printf("\n");


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
  
  /* Et on met son numéro dans nbarchiv_a_appeler                  */
  
  message.msg_type=1; //1>nbarchiv_a_appeler
  message.operation=operation;
  message.num_journaliste=numjournaliste;
  
  switch(operation){
  case CONSULTATION:
    message.theme=atoi(argv[3]);
    message.num_article=atoi(argv[4]);
    break;
  case EFFACEMENT:
    message.theme=atoi(argv[3]);
    message.num_article=atoi(argv[4]);
    break;
  case PUBLICATION:
    message.theme=atoi(argv[3]);
    strcpy(message.msg_text,argv[4]);
    break;
  }
  if (msgsnd(id_filemessage,&message,nbarchiv_a_appeler,IPC_NOWAIT)==-1){
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
}
