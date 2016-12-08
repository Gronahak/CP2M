#include "types.h"

void animation_coup_de_balai(){
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

    //    for (k=0;k<i;k++){
      printf(" ");  fflush(stdout);
      //}
    //   printf("\e[1m");
    // printf("\e[31");
  printf("/");
  fflush(stdout);

  nanosleep(&tim,NULL);
  printf("\b");  fflush(stdout);
  //  for (k=0;k<i;k++) {printf("\b");  fflush(stdout);}
  fflush(stdout);
  nanosleep(&tim,NULL);
  
  printf("\\");
  fflush(stdout);
  nanosleep(&tim,NULL);
  printf("\b");  fflush(stdout);
      printf(" ");  fflush(stdout);
  
  //      for (k=0;k<i;k++)
  //	printf("\b");
  fflush(stdout);
  i++;
  }
  //  printf("\e[0m");
    printf("\033[0m\n");

}

void arret_brutal(int s){
  int i;
  printf("\n");
  //  animation_coup_de_balai();
  FILE *fich_cle;
  char clef_lu[50];
  int id_sem;
  printf("Coup de balai dans les IPC.\n");
  
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("fich inexistant\n");
    exit(-1);
    }
  fgets(clef_lu,50,fich_cle);
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
  
  fgets(clef_lu,50,fich_cle);
  
  int nb_th,id_shm;
  nb_th=atoi(clef_lu);
  for (i=0;i<nb_th;i++){
    fgets(clef_lu,50,fich_cle);
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

  int id_fm;
  while (fgets(clef_lu,50,fich_cle)){
    id_fm=msgget(atoi(clef_lu),0660);
    msgctl(id_fm,IPC_RMID,NULL);
  }
  
  printf("Coup de balai fini\n");
  fclose(fich_cle);
  exit(1);
  
}
void mon_sigaction(int signal, void(*f)(int)){
  struct sigaction action;
  action.sa_handler=f;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;
  sigaction(signal,&action,NULL);
}

void usage(const char* chaine){
  printf("Usage : %s <nb_archivistes> <nb_themes>\n",chaine);
}

int main (int argc, char *argv[]){

  srand(getpid());
  mon_sigaction(SIGINT,arret_brutal);
  
  // pause();
  if (argc!=3){
    usage(argv[0]);
  }
  int nb_archivistes=atoi(argv[1]);
  int nb_themes=atoi(argv[2]);
  key_t cle_sem;
  key_t cle_smp;
  FILE *fich_cle;
  char cle_sem_chaine[100]={'\0'};
  char clef_filemess[100]={'\0'};
  char nb_themes_chaine[5]={'\0'};
  int clef_filemessage,id_filemessage;
  int id_ens_sem_redacteurs_prio;
  struct sembuf V={0,+1,SEM_UNDO};    

  int id_smp;
  
  int i;
  pid_t p;

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
    fich_cle = fopen(FICHIER_CLE,"r");
    if (fich_cle==NULL){
        if (errno==ENOENT){
            /* on le cree                                   */
                fich_cle=fopen(FICHIER_CLE,"w");
                if (fich_cle==NULL){
                    printf("Lancement serveur impossible\n");
                    exit(-1);
                }
        }
        else{/* Autre probleme                              */
            printf("Lancement serveur impossible\n");
            exit(-1);
        }
    }

            /* 2 - Creation proprement dite                     */

    cle_sem = ftok(FICHIER_CLE,LETTRE_CODE);
    sprintf(cle_sem_chaine,"%d",cle_sem);
    fputs(cle_sem_chaine,fich_cle);
    fputc('\n',fich_cle);
    sprintf(nb_themes_chaine,"%d",nb_themes);
    fputs(nb_themes_chaine,fich_cle);
    fputc('\n',fich_cle);
    /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                          Creation des IPC                          */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/
    
    /* 1- Creation de l'ensemble de sémaphores qui contient 5 sémaphores : */
    /*  × 0 mutex_redacteurs                                               */
    /*  × 1 lecture                                                        */
    /*  × 2 ecriture                                                       */
    /*  × 3 avant                                                          */
    /*  × 4 nombre                                                         */


    if ((id_ens_sem_redacteurs_prio=semget(cle_sem,5,IPC_CREAT|IPC_EXCL|0660))==-1) {
      printf("Echec creation mutex redacteurs\n");
      perror("mutex_redacteurs fail");
    }
    perror("1");


    printf("gneeee %d\n",semop(id_ens_sem_redacteurs_prio,&V,1));
    perror("1");
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
            printf("C'est la pause %d\n",semctl(id_ens_sem_redacteurs_prio,0,GETVAL));

    /* 2- Creation des segments de memoire partagée (1 par thème)      : */
    for (i=0;i<nb_themes;i++){
      cle_smp=ftok(FICHIER_CLE,'a'+i+1);
      char cle_smp_chaine[50]={'\0'};
      if ((id_smp=shmget(cle_smp,NB_MAX_ARTICLES*4,IPC_CREAT|IPC_EXCL|0666))==-1)
      {
	printf("Création du segment de mémoire partagée impossible\n");
	exit(1);
      }
      printf("%xd\n",cle_smp);
      sprintf(cle_smp_chaine,"%d",cle_smp);
      fputs(cle_smp_chaine,fich_cle);
      sprintf(cle_smp_chaine,"\n");
      fputs(cle_smp_chaine,fich_cle);


    }

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
        
      clef_filemessage=ftok("archiviste.c",i); //i numero d'ordre
      if ((id_filemessage=msgget(clef_filemessage,IPC_CREAT | IPC_EXCL | 0660))==-1){
	fprintf(stderr,"Probleme dans la création de la file de message de l'archiviste n°%d.\n",i);
	exit(-1);
      }

      sprintf(clef_filemess,"%d",clef_filemessage);
      fputs(clef_filemess,fich_cle);
      sprintf(clef_filemess,"\n");
      fputs(clef_filemess,fich_cle);
    }

    
    fclose(fich_cle);
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
      if (rand_requete>1)
	categorie_requete=PUBLICATION;
      if (rand_requete>3)
	categorie_requete=CONSULTATION;

      printf("dont la catégorie est: %c||%d\n",categorie_requete,rand_requete);
    
}
    pause();
    
    return 0;

}
