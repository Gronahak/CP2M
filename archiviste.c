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
  int i, clef_filemessage;
  struct tampon* message=(struct tampon*)malloc(sizeof(struct tampon));
  struct tampon* message_envoi=(struct tampon*)malloc(sizeof(struct tampon));
  char* contenu;
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
  if (fgets(id_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
  //a faire

  if ( fgets(id_lu,50,fich_cle)==NULL) //2e a incrementer suivant le msgrcv
    fprintf(stderr,"Erreur de lecture du fichier\n");
  
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
    fprintf(stderr,"MESSAGE a recevoir\n");
    msgrcv(id_filemessage,message,sizeof(struct tampon),numero_ordre,MSG_NOERROR);
    //fprintf(stderr,"MESSAGE RECU chez archiv %d\n",numero_ordre);
    contenu=shmat(tabid_shm[message->theme],0,0);
    fprintf(stderr,"Testinfos: %d OU %s et operation %c theme %d\n",message->num_journaliste,message->msg_text,message->operation,message->theme);
    // fprintf(stderr,"NUM ARTICLE- %d",message->num_article);

    /* /!\ Ouvrir la file de message du jouraliste pour lui envoyer un message */ 
    switch(message->operation){
    case CONSULTATION: /* consulter l'article */
      if (message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	fprintf(stderr,"Numéro d'article non existant (consultation) \n");
	strcpy(message_envoi->msg_text,"ERRN");
	break;
	}
      for (i=0;i<4;i++)
	message_envoi->msg_text[i]=contenu[4*message->num_article+i];
	message_envoi->operation='c';
	printf("CONSULTATION ARTICLE %d\n",message->num_article);
      break;
    case PUBLICATION: /* publier l'article */
      /* verif semaphore theme */
      printf("TEST\n");
      
      while (contenu[indice]!='-' && indice<4*NB_MAX_ARTICLES){
	indice+=4;
	numarti++;
      }
      if(indice==4*NB_MAX_ARTICLES){
	printf("ERREUR on peut plus publier\n");
	break;
      }
      for (i=0;i<4;i++)
	contenu[4*numarti+i]=message->msg_text[i];
      message_envoi->operation='p';
      fprintf(stdout,"Article %d indice %d publié MAGUEULE %s\n",numarti,indice,contenu);
	    
      break;
    case EFFACEMENT: /* supprimer l'article */
      /* verif semaphore theme */
      if(message->num_article>=NB_MAX_ARTICLES || contenu[4*message->num_article]=='-'){
	printf("erreur effacement impossible \n");
	break;
      }
      for (i=0;i<4;i++)
	contenu[4*message->num_article+i]='-';
      fprintf(stdout,"Article %d supprimé BOYAH %s \n",message->num_article,contenu);
      message_envoi->operation='e';
      break;
    default: break;
    }
    /* On previent le journaliste de l'action */
    message_envoi->msg_type=message->num_journaliste;
    if (msgsnd(id_filemessage,message_envoi,sizeof(struct tampon),0)==-1)

    shmdt(&tabid_shm[message->theme]);
    fprintf(stderr,"MESSAGE traité !!!!!!!!!\n\n");
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



