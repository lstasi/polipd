#include "polip.h"

/*
Write conf recive the strut for the config and fill the /etc/polip.conf
there is 2 whiles one for groups and the other for clients
Write Conf Recibe como parametros las tres estructuras de la configuracion y
genera el archivo de configuracion /etc/polip.conf
Tiene basicamente 2 while, uno recorre la lista de los grupos y el otro la lista
de los clientes.
*/
int writeconf (struct root *ptr,struct grupos *ptg,struct clientes *ptc)
{
 FILE *confile;
 struct grupos *tempgrp;
 struct clientes *tempcli;
 
if((confile=fopen("/etc/polip.conf","w"))==NULL)
  {
   return -1;
  }

	fprintf(confile,"\
#Config file for polipd\n\
#This file must be in /etc/polip.conf\n\
#The parser is not finish so try to mantain the structure of the file\n\
#Netdevice is the local network device\n\
#Linkdevice is the internet device\n\
#The parent of a group or client must be before them\n\
#Borrow means it can use the bandwhit not used by other clients\n\
#\n\
#Archivo de configuracion para polip\n\
#Este archivo de estar en /etc\n\
#El parser del archivo de configuracion no esta totalmente terminado por lo cual\n\
#es recomendable mantener la estructura que se muestra sin dejar espacion en blanco\n\
#Netdevice es el dispositivo conectado a la red local\n\
#Likndevice es el dispositivo conectado a internet no hace falta que este conectado siempre\n\
#Los clientes o grupos con parent que no exista no seran cargados\n\
#No colocar clientes o grupos con parent que estaran despues de los mismos\n\
#Borrow significa que este cliente o grupo puede pedir prestado ancho de banda a otros del mismo nivel\n\
#Se pueden cargar clientes a root sin crear ningun grupo\n\
\n\
#kbps Kilobyte per second\n\
#mbps Megabyte per second\n\
#kbit Kilobits per second\n\
#mbit Megabits per second\n\
#bps  Bits per second\n\
#\n"); 

//Write Root

	fprintf(confile,"\
#\n\
###Root Structure\n\
#\n\
[root]\n\
netdevice=%s\n\
netspeed=%s\n\
linkdevice=%s\n\
linkspeed=%s\n\
#\n",ptr->netdevice,ptr->netspeed,ptr->linkdevice,ptr->linkspeed);

//Write Grupos

tempgrp=ptg;

while(tempgrp!=NULL)
{
	 fprintf(confile,"\
#\n\
#Grupo\n\
#\n\
[grupo]\n\
name=%s\n\
parent=%s\n\
rate=%s\n\
borrow=%s\n\
\n\
\n",tempgrp->name,tempgrp->parent,tempgrp->rate,tempgrp->borrow);

	tempgrp=tempgrp->nxt;

}

tempcli=ptc;

while(tempcli!=NULL)
{
	 fprintf(confile,"\
#\n\
#Cliente\n\
#\n\
[Cliente]\n\
name=%s\n\
parent=%s\n\
rate=%s\n\
borrow=%s\n\
ip=%s\n\
\n\
\n",tempcli->name,tempcli->parent,tempcli->rate,tempcli->borrow,tempcli->ip);

	tempcli=tempcli->nxt;
}

fclose(confile);  
  
return 0;

}//Fin write
