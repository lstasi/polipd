#include "polip.h"
/*
Stats is a parser of the output of tc -s class show to get the speed
Stats es llamada por la funcion Server para parear las estadisticas de 
la salida de tc -s class show
*/
struct clientes *searchH (struct clientes *ptrcli,int handle)
{
  struct clientes *tmpcli;
  tmpcli=ptrcli;
	    
	    while(tmpcli!=NULL)
	      {
	       if(tmpcli->handle==handle)
	         break;
	       else
	         tmpcli=tmpcli->nxt;
	      }
	  
	   return tmpcli;
}


void stats(struct clientes *ptrcli,struct root *ptrroot)
{
 
 char tmp[128],*linea;
 struct clientes *tmpcli;
 FILE *salida;

 linea=(char *)malloc(165);
 
 sprintf(tmp,"tc -s class show dev %s",ptrroot->netdevice);
 salida=popen(tmp,"r");

 do{
	if(strncmp(linea,"class",5)==0){
	
		if((tmpcli=searchH(ptrcli,(int)strtol(strchr(linea,':')+1,NULL,0)))!=NULL){
			
			fgets(linea,160,salida);
			fgets(linea,160,salida);
			
			if(strncmp(linea," rate",5)==0){
				
				strcpy(linea,linea+6);
							
				if(strchr(linea,' ')!=NULL)
					*(strchr(linea,' '))='\0';
				else if (strchr(linea,'\n')!=NULL)
				        *(strchr(linea,' '))='\0';
				else
				        strcpy(linea,"0");
				
				tmpcli->speed=rate(linea)/8;
			}
			else
				tmpcli->speed=0;
		}
	}
 
 }while(fgets(linea,160,salida)!=NULL);
 pclose(salida);

}

