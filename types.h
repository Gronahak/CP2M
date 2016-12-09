#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <time.h>

#define FICHIER_CLE "cle.serv" // Fichier servant à stocker les clefs des IPC utilisées
#define LETTRE_CODE 'a'

#define NB_MAX_ARTICLES 10
#define NB_MAX_JOURNALISTES 1000

#define EFFACEMENT 'e'
#define PUBLICATION 'p'
#define CONSULTATION 'c'

int nombre_redacteurs; // Variables globales pour l'execution du processus avec priorité aux redacteurs 
int nombre_lecteurs;

struct tampon{
  long msg_type;
  char msg_text[5];
  char operation;
  int num_journaliste;
  int theme;
  int num_article;
};
