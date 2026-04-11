#include "polip.h"
/*
ExitClean clear the qdisc along with the filters from the device before exit
only if it is exited with SIGTERM or SIGINT not SIGKILL

Exit Clean elimina la disciplina de cola junto con los filtros al salir por SIGTERM o SIGINT
Si sale por SIGKILL la disciplina de cola queda configurada.
*/

char rootdev[30];

void ExitClean(int signum){
 
 char command[60];
 sprintf(command,"tc qdisc  del dev %s root 2>/dev/null",rootdev);
 system(command);
 exit(0);
}

int main (int argc, char **argv)
{
 
  char command[60];
  
  struct root **ptr,*ptroot=NULL;
  struct grupos **ptg,*ptgrp=NULL;
  struct clientes **ptc,*ptcli=NULL;
  struct sigaction CleanUP;
  poptContext optCon;
  
  struct poptOption optionsTable[] ={
	  { "daemon",'d',0,0,'d'},
	  { "clear",'c',0,0,'c'},
  	  { "test",'t',0,0,'t'},
	  { "help",'h',0,0,0},
	  {  NULL,0,0,NULL,0}
  };
  
  
  ptr=&ptroot;
  ptg=&ptgrp;
  ptc=&ptcli; 

  CleanUP.sa_handler=ExitClean;
  CleanUP.sa_flags=0;
  sigemptyset(&CleanUP.sa_mask);
  sigaddset(&CleanUP.sa_mask,SIGTERM);
  sigaddset(&CleanUP.sa_mask,SIGINT);
  
  sigaction(SIGTERM,&CleanUP,NULL);
  sigaction(SIGINT,&CleanUP,NULL);
  
  optCon = poptGetContext("polipd", argc,(const char **)argv, optionsTable,0);
  
  switch(poptGetNextOpt(optCon)){
	  case 'd':
		  //printf("\nDaemon Start\n");
		  if(fork()==0){
		    if(readconf(ptr,ptg,ptc)!=0){
			  fprintf(stderr,"Polipd Error: /etc/polip.conf");
			  exit(0);
		    }		
  		   strcpy(rootdev,ptroot->netdevice); //Para ExitCleanUP
		   setup(ptroot,ptgrp,ptcli);
		   server(ptroot,ptgrp,ptcli);
		  }
		  else{
		   exit(0);
		  }
		  break;
	  case 'c':
		  printf("\nSystem Clear\n");
		  sprintf(command,"tc qdisc  del dev %s root 2>/dev/null",ptroot->netdevice);
		  system(command);
		  break;
          case 't':
	  	  if(readconf(ptr,ptg,ptc)!=0){
			 fprintf(stderr,"Polipd Error: /etc/polip.conf");
		         exit(0);
		  }
  		  strcpy(rootdev,ptroot->netdevice); //Para ExitCleanUP
		  setup(ptroot,ptgrp,ptcli);
		  server(ptroot,ptgrp,ptcli);
		  break;
	  default:
		  printf("Polipd Help Options: \n -d --daemon\n -c --clear\n -h --help\n");
		  break;
  }
 return 0;
}
