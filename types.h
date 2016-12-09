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

#define FICHIER_CLE "cle.serv"

#define LETTRE_CODE 'a'
#define NB_MAX_ARTICLES 100
#define NB_MAX_JOURNALISTES 1000

#define EFFACEMENT 'e'
#define PUBLICATION 'p'
#define CONSULTATION 'c'

struct tampon{
  long msg_type;
  char msg_text[4];
  char operation;
  int num_journaliste;
  int theme;
  int num_article;
};
