#include "polip.h"

/*
Server mantain the statics 
it recive a "start" message in the multicast adress 230.0.0.1
or a "stop" message
And send the data in the multicast adress 230.0.0.2
both in 20002 port
Server es la funcion principal del daemon que mantiene las estadisticas
Recibe paquetes udp a la direccion multicast 230.0.0.1 con los mensajes de Start y Stop
Luego crea un paquete para enviar a la direccion multicast 230.0.0.2
Con los datos de todos los clientes de los cuales se recibio mensaje de Start
*/
/*
Busca el en la lista de clietnes el ip de un paqute udp recibido.
*/

struct clientes *searchIP (struct clientes *ptrcli,struct sockaddr_in IP)
{
  struct clientes *tmpcli;
  tmpcli=ptrcli;
	    while(tmpcli!=NULL)
	      {
	       if(strcasecmp(tmpcli->ip,inet_ntoa(IP.sin_addr))==0)
	        {
		 break;
		}
	       else
	         tmpcli=tmpcli->nxt;
	      }
	  
	   return tmpcli;
}

int countclients(struct clientes *ptrcli)
{
 struct clientes *tmpcli;
 int cli;
 tmpcli=ptrcli;
 cli=0;
	    while(tmpcli!=NULL)
	      {
	 	cli++;    
	        tmpcli=tmpcli->nxt;
	      }
 if(cli==0)
 	cli=1; 		//Si no hay clientes invento uno para evitar que explote malloc
 return cli;
}


int server(struct root *ptrroot,struct grupos *ptrgrp,struct clientes *ptrcli)
{

 
 int socketin,socketout,rst;
 struct timeval tiempo;
 fd_set set;
 size_t size;
 char message [64];
 struct sockaddr_in addrin,addrout;
 struct clientes *statcli;
 struct ip_mreq mcaddr;
   
 
 char tmp[128],*mensaje;
 
 mensaje=(char*)malloc(countclients(ptrcli) * 32);
 
 memset(mensaje,'\0',sizeof(*mensaje));
 
 
 memset(&addrin,0,sizeof(addrin));
 addrin.sin_family =AF_INET;
 addrin.sin_addr.s_addr =inet_addr("230.0.0.1");
 addrin.sin_port =htons(20002);

 
 memset(&addrout,0,sizeof(addrout));
 addrout.sin_family =AF_INET;
 addrout.sin_addr.s_addr =inet_addr("230.0.0.2");
 addrout.sin_port =htons(20002);
 
 mcaddr.imr_multiaddr.s_addr = inet_addr("230.0.0.1");
 mcaddr.imr_interface.s_addr = inet_addr(getipadress(ptrroot->netdevice));
 
 //set socket using SOCK_DGRAM for UDP:
 
 socketin =socket(AF_INET,SOCK_DGRAM,0);
 socketout =socket(AF_INET,SOCK_DGRAM,0);
 
 //Multicast add group
 
 rst=setsockopt(socketin,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mcaddr,sizeof(mcaddr));
 rst=bind(socketin,(struct sockaddr *)&addrin,sizeof(addrin));
 
 //Wile infinito que espera paqutes upd
 
 //Ask For Report before start
  sprintf(mensaje,"Report"); 
  rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&addrout,sizeof(addrout)); 
  memset(mensaje,'\0',sizeof(*mensaje));
  
  
 while (1)
 {
  tiempo.tv_sec=1;
  tiempo.tv_usec=0;
  FD_ZERO(&set);
  FD_SET(socketin,&set);
  
  rst=select(FD_SETSIZE,&set,NULL,NULL,&tiempo);
  
  
  if(rst==1)					//Recibi datos
  {
	    size=sizeof(addrin);
	    recvfrom(socketin,message,64,0,(struct sockaddr *)&addrin,&size);

	    statcli=searchIP(ptrcli,addrin);
	   
	    if (statcli!=NULL) //Debe ser una ip valida
	    { 
	      
	      if (strncmp(message,"Start",5)==0) //El paquete llega con el mensaje start
	        {
	         statcli->pstats=1;
		 statcli->tstats=time(NULL);
	        }
	      else if (strncmp(message,"Stop",4)==0) //Si llega un mensaje de stop.
	        {
		 statcli->pstats=0;
		}
	     }
   }
   
   //Busco estadisticas 
   memset(mensaje,'\0',sizeof(*mensaje));
   
   statcli=ptrcli;
   
   while(statcli!=NULL){          //Busco si algun cliente necesita estadisticas
	if(statcli->pstats==1){
	   break;
    	}
	statcli=statcli->nxt;
   } 
   
   
   
   if(statcli!=NULL) //Empiezo desde el primero que encontre en el While anterior
   {
      memset(mensaje,'\0',sizeof(*mensaje));
      sprintf(mensaje,"%d ",rate(ptrroot->linkspeed)/8);
      stats(ptrcli,ptrroot);
      
      while(statcli!=NULL) //Busco los clientes que quieren estadisticas y armo el paquete
	   {
	    if(statcli->pstats==1)
	    {
	     if(difftime(time(NULL),statcli->tstats)>600){
		//printf("DifTime: %d",difftime(time(NULL),statcli->tstats));
		//If has been 5 min ask for report
		sprintf(mensaje,"Report"); 
	        rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&addrout,sizeof(addrout)); 
		memset(mensaje,'\0',sizeof(*mensaje));
	        statcli->pstats=0;
	     }
	     else{
		     sprintf(tmp,"%s/%d/%d/ ",statcli->ip,statcli->speed,rate(statcli->rate)/8);
		     strcat(mensaje,tmp);
	     }
	    }
	   statcli=statcli->nxt;
	   }

    if(strlen(mensaje)>0)
     {
      rst=sendto(socketout,mensaje,strlen(mensaje),0,(struct sockaddr *)&addrout,sizeof(addrout)); 
      //printf("\n%s,Envie %d",mensaje,rst);
      //fflush(stdout);
     }
  }
	    
 }//While

}
