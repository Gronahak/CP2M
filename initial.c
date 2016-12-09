#include "types.h"

void animation_coup_de_balai(){ /* Afficher le coup de balai (fonction purement cosmétique) */
  int i=0;
  int k=0;
  fflush(stdout);
  printf("\x1b[32m\n");
  struct timespec tim;
  tim.tv_sec=0;
  tim.tv_nsec=150000000;
  for (k=0;k<=28;k++){     
    printf(".");  fflush(stdout);
  }
  printf(" \b");
  while(i<28){
    printf("\b");  fflush(stdout);
    printf("\b");  fflush(stdout);
    printf("\b");  fflush(stdout);
    printf(" ");  fflush(stdout);
    printf("/");
    fflush(stdout);
    nanosleep(&tim,NULL);
    printf("\b\a");  fflush(stdout);
    fflush(stdout);
    nanosleep(&tim,NULL);  
    printf("\\");
    fflush(stdout);
    nanosleep(&tim,NULL);
    printf("\b\a");  fflush(stdout);
    printf(" ");  fflush(stdout);
    fflush(stdout);
    i++;
  }
  printf("\033[0m\n");
}

pid_t *tableau_pid_archivistes;
int indice_tab_a=0;

pid_t *tableau_pid_journalistes;
int indice_tab_j=0;


void arret_brutal(int s){
  int i;
  int statut;
  /* Eradication des archivistes et journalistes qui se balladent :*/
  for (i=0;i<NB_MAX_JOURNALISTES;i++){
    //    printf("j%d : %d\n",i,tableau_pid_journalistes[i]);
    if (tableau_pid_journalistes[i]!=0) {
      kill(tableau_pid_journalistes[i],SIGUSR1);
      waitpid(tableau_pid_journalistes[i],&statut,WCONTINUED|WUNTRACED);
    }
  }

  for (i=0;i<indice_tab_a;i++){
    printf("    J'envoie SIGUSR1 à l'archiviste %d\n",tableau_pid_archivistes[i]);
     kill(tableau_pid_archivistes[i],SIGUSR1);
     waitpid(tableau_pid_archivistes[i],&statut,WCONTINUED|WUNTRACED);

     //printf("a%d : %d\n",i,tableau_pid_archivistes[i]);
  }

  printf("\n");
  animation_coup_de_balai();
  FILE *fich_cle;
  char clef_lu[50];
  int id_sem;
  printf("Coup de balai dans les IPC.\n");
  
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("fich inexistant\n");
    exit(-1);
  }
  if (fgets(clef_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
  id_sem=atoi(clef_lu);
  printf("<<<%s\n",clef_lu);
  printf(">>%d\n",id_sem);
  if ((id_sem=semget(id_sem,0,0))==-1){
    printf("recup impossible\n");
    perror("ohnon");
  }
  if (semctl(id_sem,IPC_RMID,0)==-1){
    printf("Destruction de l'ensemble de semaphores impossible\n");
    perror("wtf;");
    exit (-1);
  }

  if (fgets(clef_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
  id_sem=atoi(clef_lu);
  printf("<<<%s\n",clef_lu);
  printf(">>%d\n",id_sem);
  if ((id_sem=semget(id_sem,0,0))==-1){
    printf("recup impossible\n");
    perror("ohnon");
  }
  if (semctl(id_sem,IPC_RMID,0)==-1){
    printf("Destruction de l'ensemble de semaphores impossible\n");
    perror("wtf;");
    exit (-1);
  }

  int id_fm;
  if (fgets(clef_lu,50,fich_cle)==NULL)
    fprintf(stderr,"Erreur de lecture du fichier\n");
  id_fm=msgget(atoi(clef_lu),0);
  msgctl(id_fm,IPC_RMID,NULL);
  
  
  int id_shm;
  while(fgets(clef_lu,50,fich_cle)){
    printf("<<<<<<<%s\n",clef_lu);
    id_shm=atoi(clef_lu);
    if ((id_shm=shmget(id_shm,0,0))==-1){
      printf("recup impossible\n");
      perror("ohnon");
    }
    if (shmctl(id_shm,IPC_RMID,NULL)==-1){
      printf("Destruction du shm impossible\n");
      perror("wtf;");
      exit (-1);
    }
  }
  
  printf("Coup de balai fini\n");
  fclose(fich_cle);
  wait(NULL);
  exit(1);
  
}
void mon_sigaction(int signal, void(*f)(int)){
  struct sigaction action;
  action.sa_handler=f;
  sigemptyset(&action.sa_mask);
  action.sa_flags=SA_NOCLDSTOP;
  sigaction(signal,&action,NULL);
}

void usage(const char* chaine){
  printf("Usage : %s <nb_archivistes> <nb_themes>\n\t les deux paramètres doivent être >= 2\n",chaine);

}

int main (int argc, char *argv[]){

  extern int nombre_redacteurs;
  extern int nombre_lecteurs;
  
  nombre_redacteurs=0;
  nombre_lecteurs=0;

  srand(getpid());
  // pause();
  if (argc!=3){
    usage(argv[0]);
    exit(-1);
  }
  int nb_archivistes=atoi(argv[1]);
  int nb_themes=atoi(argv[2]);
  
  if (nb_archivistes<2||nb_themes<2){
    usage(argv[0]);
    exit(-1);
  }

  mon_sigaction(SIGINT,arret_brutal);
   mon_sigaction(SIGUSR1,arret_brutal);
  mon_sigaction(SIGUSR2,arret_brutal);
  mon_sigaction(SIGTERM,arret_brutal);
  mon_sigaction(SIGSTOP,arret_brutal);
  mon_sigaction(SIGHUP,arret_brutal);

  key_t cle_sem_R_P,cle_sem_file;
  key_t cle_smp;
  FILE *fich_cle;
  char cle_sem_R_P_chaine[100]={'\0'};
  char cle_sem_file_chaine[100]={'\0'};

  char clef_filemess[100]={'\0'};
  //  char nb_themes_chaine[5]={'\0'};
  int clef_filemessage,id_filemessage;
  int id_ens_sem_redacteurs_prio;
  int id_ens_sem_files_archi;
  //  struct sembuf V={0,+1,SEM_UNDO};    

  int id_smp;
  
  
  int i;
  pid_t p;
  if ((tableau_pid_archivistes=(pid_t *)malloc(nb_archivistes*(sizeof(pid_t))))==NULL){
    printf("Echec du malloc TABarchivistes\n");
  }
  if ((tableau_pid_journalistes = (pid_t *)malloc(NB_MAX_JOURNALISTES*(sizeof(pid_t))))==NULL){
    printf("Echec du malloc TABjournalistes\n");
  }

  int rand_requete,categorie_requete;
  /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                     Creation du fichier cle                        */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/
      

  /* Creation de la cle :                                 */
  /* 1 - On teste si le fichier cle existe dans 
     le repertoire courant : 
  */
  fich_cle = fopen(FICHIER_CLE,"w"); //sinon r
  // if (fich_cle==NULL){
  //if (errno==ENOENT){
      /* on le cree                                   */
  //  fich_cle=fopen(FICHIER_CLE,"w");
      if (fich_cle==NULL){
	printf("Lancement serveur impossible\n");
	exit(-1);
      }
      //   }
      //  else{/* Autre probleme                              */
  // printf("Lancement serveur impossible\n");
      //   exit(-1);
      //   }
      // }

  /* 2 - Creation proprement dite                     */

  cle_sem_R_P = ftok(FICHIER_CLE,LETTRE_CODE);
  sprintf(cle_sem_R_P_chaine,"%d",cle_sem_R_P);
  fputs(cle_sem_R_P_chaine,fich_cle);
  fputc('\n',fich_cle);

  printf("\tcle de l'es redac prio : %xd \n",cle_sem_R_P);


  /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                          Creation des IPC                          */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/
    
  /* 1- Creation de l'ensemble de sémaphores qui contient 4+nb themes sémaphores : */
  /*  × 0 mutex_redacteurs                                                         */
  /*  × 1 mutex_lecture                                                            */
  /*  × 2 mutex_avant                                                              */
  /*  × 3 mutex_nombre                                                             */
  /*  × 4-n autant de mutex qu'il y a de themes                                    */
  /* Cet ensemble de sémaphores sert à réglementer le protocole lecteur            */
  /* rédacteurs avec priorité aux rédacteurs                                       */


  if ((id_ens_sem_redacteurs_prio=semget(cle_sem_R_P,4+nb_themes,IPC_CREAT|IPC_EXCL|0660))==-1) {
    printf("Echec creation ES redacteurs prio\n");
    perror("ES redacteurs_prio fail");
  }
  //    perror("1");


  //    printf("gneeee %d\n",semop(id_ens_sem_redacteurs_prio,&V,1));
  //    perror("1");
  /*
    semop(id_ens_sem_redacteurs_prio,&V,2);
    perror("2");
    semop(id_ens_sem_redacteurs_prio,&V,3);
    perror("3");
    semop(id_ens_sem_redacteurs_prio,&V,4);
    perror("4");
    semop(id_ens_sem_redacteurs_prio,&V,0);
    perror("5");
  */
  //  printf("C'est la pause %d\n",semctl(id_ens_sem_redacteurs_prio,0,GETVAL));




  /* 2- Creation de l'ensemble de sémaphores qui contient                */
  /*  nb_archivistes + 1  sémaphores                                     */
  /*    × 1 sem pour chaque archi qui dit la taille de la file           */
  /*    × 1 sem mutex pour que les journalistes modifient proprement     */
  /*      la taille des files                                            */
  /* Cet ensemble de sémaphores sert à connaitre en temps réel quel est  */
  /* l'archiviste le moins occupé                                        */

  cle_sem_file = ftok(FICHIER_CLE,LETTRE_CODE+1);
  sprintf(cle_sem_file_chaine,"%d",cle_sem_file);
  fputs(cle_sem_file_chaine,fich_cle);
  printf("\tcle de l'es taille des files : %xd \n",cle_sem_file);
  if ((id_ens_sem_files_archi=semget(cle_sem_file,1+nb_archivistes,IPC_CREAT|IPC_EXCL|0660))==-1) {
    printf("Echec creation ES file_archi\n");
    perror("ES file_archi fail");
  }
  fputc('\n',fich_cle);

  /************* initialisation des Ensembles de sémaphores **************/

  printf("DEBUUUUG1\n");
  ushort *tab;
  tab = (ushort*)malloc((4+nb_themes)*sizeof(ushort));
  if (tab==NULL){printf("Y'a plus de memoiren abandonnez le navire.\n") ;exit(-1);}
  for (i=0;i<4+nb_themes;i++)tab[i]=1;
  ushort tab2[NB_MAX_JOURNALISTES]={0};

  printf("DEBUUUUG2\n");
  for (i=0;i<NB_MAX_JOURNALISTES;i++)tab2[i]=0;
  tab2[0]=1;

  printf("DEBUUUUG3\n");
  //  printf("\x1b[32m\n");
  
  if((semctl(id_ens_sem_redacteurs_prio,0,SETALL,tab)) ==-1){printf("ça déconne\n");perror("semctl1:");}
  if((semctl(id_ens_sem_files_archi,0,SETALL,tab2)) ==-1){printf("ça déconne\n");perror("semctl1:");}
  /*  printf("val semaphore : \n");
  if((semctl(id_ens_sem_redacteurs_prio,5,GETALL,tab)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
  for (i=0;i<5;i++)printf("%d |",tab[i]);

  printf("\n");
  int valeurhihi=6;
  if((semctl(id_ens_sem_files_archi,2,SETVAL,valeurhihi)) ==-1){printf("ça déconne2\n");perror("semctl2:");}
    if((semctl(id_ens_sem_files_archi,5,GETALL,tab)) ==-1){printf("ça déconne2\n");perror("semctl2:");}

    for (i=0;i<5;i++)printf("%d |",tab[i]);
  
  
  printf("\033[0m\n");
  */

  /***********************************************************************/
    
  /* 3- Creation de la file de message                                    */
  clef_filemessage=ftok("archiviste.c",10);
  if ((id_filemessage=msgget(clef_filemessage,IPC_CREAT | IPC_EXCL | 0660))==-1){
    fprintf(stderr,"Probleme dans la création de la file de message.\n");
    exit(-1);
  }
  printf("\tcle de la fm: %xd\n",clef_filemessage);
  sprintf(clef_filemess,"%d",clef_filemessage);
  fputs(clef_filemess,fich_cle);
  fputc('\n',fich_cle);
    
  /* 4- Creation des segments de memoire partagée (1 par thème)      : */
  fprintf(stdout," nb themes: %d\n",nb_themes);
  for (i=0;i<nb_themes;i++){
    cle_smp=ftok(FICHIER_CLE,'a'+i+2);
    char cle_smp_chaine[50]={'\0'};
    char* contenu, init[NB_MAX_ARTICLES*4];
    if ((id_smp=shmget(cle_smp,NB_MAX_ARTICLES*4,IPC_CREAT|IPC_EXCL|0666))==-1)
      {
	printf("Création du segment de mémoire partagée impossible\n");
	exit(1);
      }
    int j;
    for(j=0;j<NB_MAX_ARTICLES*4;j++)
      init[j]='-';
    contenu=shmat(id_smp,0,0);
    strcpy(contenu,init);
    shmdt(&contenu);
    printf("\tcle du smp %d : %xd\n",i,cle_smp);
    sprintf(cle_smp_chaine,"%d",cle_smp);
    fputs(cle_smp_chaine,fich_cle);
    fputc('\n',fich_cle);
  }

    
  fclose(fich_cle);  
  
  /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                   Creation des Archivistes                         */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/

  
    
  fprintf(stderr,"Création des archivistes");
  for(i=1;i<=nb_archivistes;i++){
    fprintf(stderr,".");
    if ((p=fork())==-1){
      printf("Echec du fork\n");
      exit(-1);
    }
    else if (p==0){ /* Code du fils */
      char * argexecve[]={NULL,NULL,NULL,NULL};
      argexecve[0]="archiviste";
      argexecve[2]=strdup(argv[2]);
      char arg2[3]={'\0'};
	    
      sprintf(arg2,"%d",i);
      argexecve[1]=arg2;
      execve("./archiviste",argexecve,NULL);
      exit(-1);
    }
    tableau_pid_archivistes[indice_tab_a++]=p;
  }
    
    
  /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                   Creation des Journalistes                        */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/
    

  while (1){
    sleep(1);
    printf("Création d'un journaliste\n");
    rand_requete=rand()%10+1;

    categorie_requete=EFFACEMENT;
    if (rand_requete>1)  //1
      categorie_requete=PUBLICATION;
    if (rand_requete>3)  //3
      categorie_requete=CONSULTATION;

    printf("dont la catégorie est: %c||%d\n",categorie_requete,rand_requete);



    if ((p=fork())==-1){
      printf("Echec du fork\n");
      exit(-1);
    }
    else if (p==0){ /* Code du fils */
      char * argexecve[]={NULL,NULL,NULL,NULL,NULL,NULL};
      argexecve[0]="journaliste";
      argexecve[1]=strdup(argv[1]);
      char arg2[2]={'\0'};
      arg2[0]=categorie_requete;
      argexecve[2]=arg2;
      int theme;
      theme=rand()%nb_themes;
      char arg3[2]={'\0'};
      sprintf(arg3,"%d",theme);
      argexecve[3]=arg3;
      int numero_article;

      char article[5];
      switch (categorie_requete){
      case CONSULTATION:
      case EFFACEMENT:
	  
	numero_article=rand()%NB_MAX_ARTICLES;
	sprintf(article,"%d",numero_article);
	argexecve[4]=article;
	break;
      case PUBLICATION:
	  

	article[0]='A'+rand()%26;
	article[1]='a'+rand()%26;
	article[2]='a'+rand()%26;
	article[3]='a'+rand()%26;
	article[4]='\0';
	  
	  
	argexecve[4]=article;
	  
	break;
	  
      }
      execve("./journaliste",argexecve,NULL);
	  
      // exit(-1);
    }
      tableau_pid_journalistes[indice_tab_j%NB_MAX_JOURNALISTES]=p;
      indice_tab_j++;
     
  }
  pause();
    
  return 0;

}
