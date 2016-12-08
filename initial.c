#include "types.h"
void arret_brutal(int s){
}
void mon_sigaction(int signal, void(*f)(int)){
  struct sigaction action;
  action.sa_handler=f;
  sigemptyset(&action.sa_mask);
  action.sa_flags=0;
  sigaction(signal,&action,NULL);
}


int main (int argc, char *argv[]){


  return 0;

}
