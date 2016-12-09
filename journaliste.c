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
  int  clef_filemessage,id_filemessage;
  // int nb_archiviste=atoi(argv[1]);
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
  
  //  erreur=
  if (fgets(id_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
  //semaphores1 inutiles ici

  /* On recupere l'ensemble 2 de semaphore */
  //  erreur=
  if (  fgets(id_lu,50,fich_cle)==NULL) // semaphore2 a traiter
    fprintf(stderr,"Erreur de lecture du fichier\n");
  /*Lancement file de message */
      
  //  erreur=
  if (fgets(id_lu,50,fich_cle)==NULL) // archiviste
    fprintf(stderr,"Erreur de lecture du fichier\n");
  clef_filemessage=atoi(id_lu);
  
  if ((id_filemessage=msgget(clef_filemessage,0))==-1){ 
    fprintf(stderr,"Probleme dans la recuperation de la file de message.\n");
    perror("erreur: ");
    exit(-1);
  }
  
  fclose(fich_cle);
  /* On cherche quel archiviste a le moins de message dans sa file avec les semaphores */
  
  /* Et on met son numéro dans nbarchiv_a_appeler                  */
  message->msg_type=1; //1>nbarchiv_a_appeler
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
  
  if (msgsnd(id_filemessage,message,sizeof(struct tampon),0)==-1){
    fprintf(stderr,"Erreur d'envoi d'un message à l'archiviste %d.\n",nbarchiv_a_appeler);
    perror("erreur: ");
  }
    
  /* On attend le message à recevoir */
  while(1){
    if ((id_message=msgrcv(id_filemessage,message_recu,(sizeof(struct tampon)),getpid(),MSG_NOERROR))>0){
      if (message_recu->operation=='c')
	printf("Message reçu article:%s, je meurs !\n",message_recu->msg_text);
      printf("Message reçu chez le journaliste, je meurs !\n");
      break;
    }
  }
  
  exit(0);
}
