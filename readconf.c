#include "polip.h"
/*
Read the configuraction from /etc/polip.conf

Lee la configuracion de /etc/polip.conf
Cada vez que se encuentra un nuevo grupo o cliente se agrega una nueva
estructura a la lista utilizando malloc
Se va buscando labels en el archvio, una vez que se encuentra uno se parsean
todos los campos en orden
los labels son
[root]
[cliente]
[grupo]
Todo lo que tenga # es un comentario y se elimina
Se leen lineas de hasta 200 caracteres
*/

int readconf (struct root **ptr,struct grupos **ptg,struct clientes **ptc)
{
  FILE *confile;
  char templine[200];
  struct grupos *tempgrp=NULL;
  struct clientes *tempcli=NULL;
  struct root *temproot=NULL;
  int handle;
   
  //Numero de identificador para las clases
  handle=1;

  // Abro el Archivo de configuracion

  if((confile=fopen("/etc/polip.conf","r"))==NULL)
  {
 	*ptr=NULL;
	*ptg=NULL;
	*ptc=NULL;
	 return -1;
  }
  else
  {
   //While para leer todas las lineas y parsearlas en la estructuras 
   
   while((fgets(&templine[0],200,confile))!=NULL)
     {
      //If para ver si es un comentario
      
      if(templine[0]!='#' && templine[0]!='\n')
      {
       //Si nos un comentario hay que analizar
       if(templine[0]=='[') //If si es un label
       {
	switch (templine[1])
	{
	 case 'R':
	 case 'r': //label root
	 
	       *ptr=(struct root *)malloc(sizeof(struct root)); 
		
		temproot=*ptr;

		fgets(&templine[0],200,confile);
        	strcpy(temproot->netdevice,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile); 
		strcpy(temproot->netspeed,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(temproot->linkdevice,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(temproot->linkspeed,cnl((strchr(&templine[0],'='))+1));
		temproot->handle=handle;
		handle=handle+3;
	   break;
	   
         case 'G':
	 case 'g': //label grupo
	 
	 		
		if(tempgrp==NULL)
		{
		    *ptg=(struct grupos *)malloc(sizeof(struct grupos)); 
		    tempgrp=*ptg;
		}
		else
		{		
		tempgrp->nxt=(struct grupos *)malloc(sizeof(struct grupos)); 
		tempgrp=tempgrp->nxt;
		}
						
		fgets(&templine[0],200,confile);
        	strcpy(tempgrp->name,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile); 
		strcpy(tempgrp->parent,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(tempgrp->rate,cnl((strchr(&templine[0],'='))+1));
		//fgets(&templine[0],200,confile);
		//strcpy(tempgrp->share,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(tempgrp->borrow,cnl((strchr(&templine[0],'='))+1));
		tempgrp->nxt=NULL;
		tempgrp->handle=handle;
		handle++;
	   break;
         case 'C':
	 case 'c': //label clientes
	 
                if(tempcli==NULL)
		{
		  *ptc=(struct clientes *)malloc(sizeof(struct clientes)); 
		  tempcli=*ptc;
		}
		else
		{	
		 tempcli->nxt=(struct clientes *)malloc(sizeof(struct clientes)); 
		 tempcli=tempcli->nxt;
		}
				
		fgets(&templine[0],200,confile);
        	strcpy(tempcli->name,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile); 
		strcpy(tempcli->parent,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(tempcli->rate,cnl((strchr(&templine[0],'='))+1));
		//fgets(&templine[0],200,confile);
		//strcpy(tempcli->share,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(tempcli->borrow,cnl((strchr(&templine[0],'='))+1));
		fgets(&templine[0],200,confile);
		strcpy(tempcli->ip,cnl((strchr(&templine[0],'='))+1));
		tempcli->pstats=0;
		tempcli->tstats=0;
		tempcli->speed=0;
		tempcli->nxt=NULL;
		tempcli->handle=handle;
		handle++;
	   break; 

	 default: //label inexistente
	     fprintf(stderr,"\nNon existing label %s",&templine[0]);
	     return -1;
	  break;

	 }//Fin Switch
      	} //Fin if label
	else
         {
          fprintf(stderr,"\nUncomment line %s",&templine[0]);
	  return -1;
         }
      }//Fin if not comment
     }//Fin While
  //Cierro Archivo de configuracion
  fclose(confile);
  return 0;
  }//else

 }
