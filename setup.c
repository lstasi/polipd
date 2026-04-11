#include "polip.h"

/*
Setup config the kernel according to polip.conf

Setup es la encargada de inicializar o configurar el kernel de acuerdo a la
informacion en polip.conf
*/


/*
Borrow parsea yes o no, que son las opciones que tiene la varibale borrow de la
estrcuctura clientes y retorna la volocidad que sera utilizada para ceil.
Si borrow es no ceil es 0
Si borrow es yes ceil es igual a la velocidad de el link de internet.
*/

char *borrow(char *s,char *speed)
{
 
 if(strcasecmp(s,"yes")==0)
  return speed;
 else
  return "0";
}


/*
Esta funcion era utilizada para CBQ

char *share(char *s)
{
 
 if(strcasecmp(s,"yes")==0)
  return "sharing";
 else
  return "isolated ";
}
*/

/*
getparetn y getparents buscan el padre de un cliente a travez de la comparacion
del string parent en el cliente con el string nombre de todos los grupos.
getparent retorna el numero o handle o clase del grupo
getparents retorna la volocidad o rate del grupo.
*/

char *getparents(char * parent,struct grupos *ptr,struct root *ptroot)
{
 char *h;
 int hangoff;
 if ((strcasecmp(parent,"root"))==0)
  {
    h=(char *)ptroot->linkspeed;
  }
 hangoff=0;

 while(ptr!=NULL)
 {
  if(strcasecmp(parent,ptr->name)==0)
  {
   h=(char *)ptr->rate;
   break;
  }
  else
    ptr=ptr->nxt;  
  
  if(hangoff>10000)
    break;
   else
    hangoff++;

  }
return h;
}//Fin getparents

int getparent(char * parent,struct grupos *ptr,struct root *ptroot)
{
 int h,hangoff;
 
 h=ptroot->handle+2;

 if ((strcasecmp(parent,"root"))==0)
  {
    h=ptroot->handle+2;
  }
 hangoff=0;

 while(ptr!=NULL)
 {
  if(strcasecmp(parent,ptr->name)==0)
  {
   h=ptr->handle;
   break;
  }
  else
    ptr=ptr->nxt;  
  
 if(hangoff>1000)
   break;
  else
   hangoff++;

 }
return h;
}//Fin getparent



int setup (struct root *ptrroot,struct grupos *ptrgrp,struct clientes *ptrcli)
{
    
    
    char command[200];
    struct grupos *tempgrp;
    struct clientes *tempcli;
    //int hangoff;
    FILE *loger;
    
    
    loger=fopen("/var/run/polipd","w");
    
    //Elimino todo lo que tenga el dispositivo 
        
    sprintf(command,"tc qdisc  del dev %s root 2>/dev/null",ptrroot->netdevice);
    fprintf(loger,"%s\n",command);
    //ejecuto el command
    system(command);
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif    
    //Creo la diciplina de cola HTB p/root
    
    sprintf(command,"tc qdisc  add dev %s root handle %d:0 htb default 1",
    	    ptrroot->netdevice,ptrroot->handle);
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif
    fprintf(loger,"%s\n",command);    
    //ejecuto el command
    system(command);
    
    //Clase para trafico por defecto
    //sprintf(command,"tc class  add dev %s parent %d:0 classid %d:%d htb rate 10 burst 0 cburst 0",
    //        ptrroot->netdevice,ptrroot->handle,ptrroot->handle,ptrroot->handle);
    //#ifdef __VERBOSE__
    //printf("\n%s\n",command);
    //#endif
    //fprintf(loger,"%s\n",command);    
    //ejecuto el command
    //system(command);
     
    //Clase Basica del resto del link de la red
    
    sprintf(command,"tc class  add dev %s parent %d:0 classid %d:%d htb rate %d",
            ptrroot->netdevice,ptrroot->handle,ptrroot->handle,(ptrroot->handle)+1,(rate(ptrroot->netspeed))-(rate(ptrroot->linkspeed)));
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif
    fprintf(loger,"%s\n",command);    
    //ejecuto el command
    system(command);
        
    //Diciplina de cola SFQ para clase del resto de la red
    
    sprintf(command,"tc qdisc  add dev %s parent %d:%d handle %d:0 sfq perturb 10",
    	    ptrroot->netdevice,ptrroot->handle,ptrroot->handle+1,ptrroot->handle+1);
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif
    fprintf(loger,"%s\n",command);    
    system(command);
      
    //Filtro para paquetes de la red interna
    
    sprintf(command,"tc filter add dev %s parent %d:0 protocol ip prio 1 u32 match ip src %s flowid %d:%d",
               ptrroot->netdevice,ptrroot->handle,getipadress(ptrroot->netdevice),ptrroot->handle,ptrroot->handle+1);
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif    
    fprintf(loger,"%s\n",command);
    //ejecuto el command
    system(command);    
    

    //Clase Basica del tamaño del link de internet

    sprintf(command,"tc class  add dev %s parent %d:0 classid %d:%d htb rate %s",
            ptrroot->netdevice,ptrroot->handle,ptrroot->handle,ptrroot->handle+2,ptrroot->linkspeed);
    #ifdef __VERBOSE__
    printf("\n%s\n",command);
    #endif
    fprintf(loger,"%s\n",command);
    //ejecuto el command
    system(command);

    //hangoff=0;
    tempgrp=ptrgrp;
        
    while(tempgrp!=NULL)
     {
      
      sprintf(command,"tc class  add dev %s parent %d:%d classid %d:%d htb rate %s ceil %s",
      		ptrroot->netdevice,ptrroot->handle,getparent(tempgrp->parent,ptrgrp,ptrroot),ptrroot->handle,
		tempgrp->handle,tempgrp->rate,borrow(tempgrp->borrow,ptrroot->linkspeed));
      #ifdef __VERBOSE__
      printf("\n%s\n",command);
      #endif 
      fprintf(loger,"%s\n",command);
      system(command);

      tempgrp=tempgrp->nxt; 
    
     }//Fin while grupos

	    
    //hangoff=0;
    tempcli=ptrcli;
    
    while(tempcli!=NULL)
     {

      //Creacion de la clase para el cliente
      sprintf(command,"tc class  add dev %s parent %d:%d classid %d:%d htb rate %s ceil %s",
            ptrroot->netdevice,ptrroot->handle,getparent(tempcli->parent,ptrgrp,ptrroot),ptrroot->handle,
	    tempcli->handle,tempcli->rate,borrow(tempcli->borrow,ptrroot->linkspeed));
      #ifdef __VERBOSE__
      printf("\n%s\n",command);
      #endif    
      fprintf(loger,"%s\n",command);
      system(command);
      //Creacion de deiciplina de cola SFQ para el cliente
      sprintf(command,"tc qdisc  add dev %s parent %d:%d handle %d:0 sfq perturb 10",
              ptrroot->netdevice,ptrroot->handle,tempcli->handle,tempcli->handle);
      #ifdef __VERBOSE__
      printf("\n%s\n",command);
      #endif    
      fprintf(loger,"%s\n",command);
      system(command);
      
      //FILTROS para los clientes
   
      sprintf(command,"tc filter add dev %s parent %d:0 protocol ip prio 1 u32 match ip dst %s flowid %d:%d",
               ptrroot->netdevice,ptrroot->handle,tempcli->ip,ptrroot->handle,tempcli->handle);
      
      //sprintf(command,"tc filter add dev %s parent %d:0 protocol ip prio 1 handle %d fw classid %d:%d",
      //		ptrroot->netdevice,ptrroot->handle,tempcli->handle,ptrroot->handle,tempcli->handle);
		
      #ifdef __VERBOSE__
      printf("\n%s\n",command);
      #endif    
      fprintf(loger,"%s\n",command);
      system(command);
      
      //sprintf(command,"iptables -A output -t mangle -d %d/32 -j MARK --set-mark %d",tempcli->ip,tempcli->handle);
      //sprintf(command,"ipchains -A output -d %s/32 -m %d",tempcli->ip,tempcli->handle);
      //printf("\n%s\n",command);
      //system(command);

      //Proxmio cliente
      tempcli=tempcli->nxt;

     }//Fin while clientes
    fclose(loger);

    return 0;
}
