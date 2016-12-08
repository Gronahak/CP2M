#include "types.h"
void arret_brutal(int s){
  FILE *fich_cle;
  char id_lu[28];
  int id_sem;
  printf("Coup de balai dans les IPC.\n");
  
  fich_cle = fopen(FICHIER_CLE,"r");
  if (fich_cle==NULL){
    printf("fich inexistant\n");
    exit(-1);
    }
  fgets(id_lu,50,fich_cle);
  id_sem=atoi(id_lu);
  if ((id_sem=semget(id_sem,0,0))==-1){
    printf("recup impossible\n");
    perror("ohnon");
  }
  if (semctl(id_sem,0,IPC_RMID)==-1){
    printf("Destruction de l'ensemble de semaphores impossible\n");
    perror("wtf;");
    exit (-1);
  }
  printf("Coup de balai fini\n");
  
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
    mon_sigaction(SIGINT,arret_brutal);

    // pause();
  if (argc!=3){
    usage(argv[0]);
  }
  int nb_archivistes=atoi(argv[1]);
  int nb_themes=atoi(argv[2]);
  key_t cle_smp;
  FILE *fich_cle;
  char cle_sem_chaine[100]={'\0'};


  int id_mutex_redacteurs;

  
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

    cle_smp = ftok(FICHIER_CLE,LETTRE_CODE);
    sprintf(cle_sem_chaine,"%d",cle_smp);
    fputs(cle_sem_chaine,fich_cle);
  /**********************************************************************/
  /*                                                                    */
  /*                                                                    */
  /*                          Creation des IPC                          */
  /*                                                                    */
  /*                                                                    */
  /*                                                                    */
  /**********************************************************************/
  
   if ((id_mutex_redacteurs=semget(cle_smp,5,IPC_CREAT|IPC_EXCL|0660))==-1) {
      printf("Echec creation mutex redacteurs\n");
      perror("mutex_redacteurs fail");
    }
  
  

   pause();
  
  return 0;

}
