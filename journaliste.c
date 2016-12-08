/*********************************************************/
/*                                                       */
/*              Programme des Journaliste                */
/*                                                       */
/*********************************************************/

# include "types.h"

/* Quatres arguments:                               */
/*      -nombre d'archivistes                       */
/*      -operation (c,p,e) +2autres/operation       */
/* c: theme && numero article                       */
/* p: theme && texte article                        */
/* e: theme && numero article                       */

int main (int argc, char *argv[]){

  int i, clef_filemessage,id_filemessage;
  int nb_archiviste=atoi(argv[1]);
  char operation=argv[2][0];
  int tabid_fm[nb_archiviste],tabclef_fm[nb_archiviste], tablongueur_fm[nb_archiviste];
  int numjournaliste=getpid(); // Ou un param ?
  int nbarchiv_a_appeler=0;
  int tmp;
  struct msqid_ds * info=(struct msqid_ds*)malloc(sizeof(struct msqid_ds));
  struct tampon message;
  char * erreur;
  FILE *fich_cle;
  char id_lu[100];
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("Fichier inexistant\n");
    exit(-1);
  }
  
  /* Lancement file de message du journaliste */
  clef_filemessage=ftok("journaliste.c",numjournaliste);
  if ((id_filemessage=msgget(clef_filemessage,IPC_CREAT | IPC_EXCL | 0660))==-1){
    fprintf(stderr,"Probleme dans la création de la file de message du journaliste n°%d.\n",numjournaliste);
    exit(-1);
  }

  /*Lancement files de message des archivistes */
  for (i=0; i<nb_archiviste; i++){
    erreur=fgets(id_lu,50,fich_cle); //semaphores inutiles ici
    erreur=fgets(id_lu,50,fich_cle);
    if (1==2) return fprintf(stderr,"%s LOL",erreur);
    tabclef_fm[i]=atoi(id_lu);
    fprintf(stdout,"test de connard %d\n",tabclef_fm[i]);
    if ((tabid_fm[i]=msgget(tabclef_fm[i],IPC_CREAT | IPC_EXCL | 0660))==-1){ 
      fprintf(stderr,"Probleme dans la création de la file de message de l'archiviste n°%d.\n",i);
      perror("erreur: ");
      exit(-1);
    }
    if (msgctl(tabid_fm[i],IPC_STAT,info)==-1){
      fprintf(stderr,"Erreur dans la consultation des infos de la file de message.\n");
      perror("erreur: ");
      exit(-1);
    }
    tablongueur_fm[i]=info->msg_qnum; /* Nombre de message dans la file */
  }
  tmp=tablongueur_fm[0];
  fclose(fich_cle);
  /* On cherche quel archiviste a le moins de message dans sa file */
  /* Et on met son numéro dans nbarchiv_a_appeler                  */
  for (i=1; i<nb_archiviste; i++){
    if (tablongueur_fm[i]<tmp){
      tmp=tablongueur_fm[i];
      nbarchiv_a_appeler=i;
    }
  }

  message.msg_type=1;
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
  if (msgsnd(tabid_fm[nbarchiv_a_appeler],&message,10,IPC_NOWAIT)==-1){
    fprintf(stderr,"Erreur d'envoi d'un message à l'archiviste %d.\n",nbarchiv_a_appeler);
    perror("erreur: ");
  }
  
  /* On supprime tous les IPC */
  
  msgctl(id_filemessage,IPC_RMID,NULL);
  
  for (i=0; i<nb_archiviste; i++)
    msgctl(tabid_fm[i],IPC_RMID,NULL);
}
