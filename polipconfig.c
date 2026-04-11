#include "polip.h"
#include <newt.h>
/*
Polipconfig is a wizard to create polip.conf

Polip config es el wizard para crear el archivo polip.conf
Tiene basicamente una interface creada con newt
Realiza mucho trabajo sobre las estructuras de configuracion mediante busquedas
inserciones y eliminaciones.
Todos los cambios solo se hacen efectivos cuando se escribe el archivo al salir
*/

char grupoactual[20]; //String global para saber que grupo estoy

void delG(struct grupos *current,struct grupos **ptg,struct clientes **ptc);//Eliminar Grupos
void delH(char *name,struct grupos **ptg,struct clientes **ptc);//Eliminar los Hijos de un grupo para la recursividad
void delC(struct clientes *current,struct clientes **ptc);//Eliminar Clientes

char * getparent(char * parent,struct grupos *ptr)
{
 int hangoff;
 
 if ((strcasecmp(parent,"root"))==0)
  return parent;
  
 hangoff=0;

 while(ptr!=NULL)
 {
  if(strcasecmp(parent,ptr->name)==0)
  {
   return ptr->parent;
   break;
  }
  else
    ptr=ptr->nxt;  
 
   
 if(hangoff>100)
   break;
  else
   hangoff++;

 }
 return "root";
}//Fin getparent

void changeH(char *old,char *new,struct grupos **ptg,struct clientes **ptc)
{
  struct grupos *tempgrp;
  struct clientes *tempcli;
  
   tempgrp=*ptg;
	      while(tempgrp!=NULL)
	      {
	       if((strcasecmp(tempgrp->parent,old)==0))
	           strcpy(tempgrp->parent,new);
	       else
	        tempgrp=tempgrp->nxt;
	      }
   tempcli=*ptc;
	    while(tempcli!=NULL)
	      {
	       if(strcasecmp(tempcli->parent,old)==0)
       	           strcpy(tempcli->parent,new);
	       else
	       tempcli=tempcli->nxt;
	      }
}

void changeC(struct clientes *tmpcli) //Cambiar Clientes
{
 struct newtExitStruct salida;
 const char **field=NULL;
 newtComponent formulario,componente[16];
 int i;
 
  newtCenteredWindow(50,21,"Clients Config");
  formulario=newtForm(NULL,NULL,0);

  componente[0]=newtTextboxReflowed(1,0,"Please set the client configuration\nUse this speeds: kbit,mbit,bps"
  					"\nBorrow is to borrow bandwith",40, 10, 10, 0);
  componente[1]=newtTextboxReflowed(4,5,"Change Client",40 , 10, 10, 0);
  componente[2]=newtLabel(5,7,"Name:");
  componente[3]=newtLabel(5,8,"Parent:");
  componente[4]=newtLabel(5,9,"Rate");
  componente[5]=newtLabel(5,10,"IP:");
  componente[6]=newtEntry(14,7,tmpcli->name,15,field,0);
  componente[7]=newtEntry(14,8,grupoactual,15,field,NEWT_FLAG_DISABLED);
  componente[8]=newtEntry(14,9,tmpcli->rate,15,field,0);
  componente[9]=newtEntry(14,10,tmpcli->ip,20,field,0);
  //if(strcasecmp(tmpcli->share,"yes")==0)
  //{
  //componente[10]=newtRadiobutton(5,12,"Sharing ",1,NULL);
  //componente[11]=newtRadiobutton(17,12,"Isolated ",0,componente[10]);
  //}
  //else
  //{
  //componente[10]=newtRadiobutton(5,12,"Sharing ",0,NULL);
  //componente[11]=newtRadiobutton(17,12,"Isolated ",1,componente[10]);
  //}
  if(strcasecmp(tmpcli->borrow,"yes")==0)
  {
  componente[10]=newtRadiobutton(5,12,"Borrow  ",1,NULL);
  componente[11]=newtRadiobutton(17,12,"Bounded  ",0,componente[10]);
  }
  else
  {
  componente[10]=newtRadiobutton(5,12,"Borrow  ",0,NULL);
  componente[11]=newtRadiobutton(17,12,"Bounded  ",1,componente[10]);
  
  }
  componente[12]=newtButton(22,16,"Done");
  componente[13]=newtButton(32,16,"Cancel");  

  i=0;
  for (i=0;i<14;i++)
    newtFormAddComponent(formulario,componente[i]);


  newtFormRun(formulario,&salida);
  
  if(salida.u.co==componente[12] || salida.reason==0)
   {
    strcpy(tmpcli->name,newtEntryGetValue(componente[6]));
    strcpy(tmpcli->parent,newtEntryGetValue(componente[7]));
    strcpy(tmpcli->rate,newtEntryGetValue(componente[8]));
    strcpy(tmpcli->ip,newtEntryGetValue(componente[9]));
    //if(newtRadioGetCurrent(componente[10])==componente[10])
    //  strcpy(tmpcli->share,"yes");
    //else
    //  strcpy(tmpcli->share,"no");
    if(newtRadioGetCurrent(componente[10])==componente[10])
      strcpy(tmpcli->borrow,"yes");  
    else
      strcpy(tmpcli->borrow,"no");  
   }
  
  newtFormDestroy(formulario);
  newtPopWindow();

}
void changeG(struct grupos *tempgrp,struct grupos **ptg,struct clientes **ptc)//Cambiar Grupos
{
 struct newtExitStruct salida;
 const char **field=NULL;
 newtComponent formulario,componente[14];
 int i;
 
  
	
  
  newtCenteredWindow(50,20,"Group Config");
  formulario=newtForm(NULL,NULL,0);
  
  componente[0]=newtTextboxReflowed(1,0,"Please set the group configuration\nUse this speeds: kbit,mbit,bps"
  					"\nBorrow is to borrow bandwith",40, 10, 10, 0);
  componente[1]=newtTextboxReflowed(4,5,"Add Group",40 , 10, 10, 0);
  componente[2]=newtLabel(5,7,"Nombre:");
  componente[3]=newtLabel(5,8,"Parent:");
  componente[4]=newtLabel(5,9,"Rate");
  componente[5]=newtEntry(14,7,tempgrp->name,15,field,0);  
  componente[6]=newtEntry(14,8,grupoactual,15,field,NEWT_FLAG_DISABLED);
  componente[7]=newtEntry(14,9,tempgrp->rate,15,field,0);
  //if(strcasecmp(tempgrp->share,"yes")==0)
  //{
  // componente[8]=newtRadiobutton(5,11,"Sharing ",1,NULL);
  // componente[9]=newtRadiobutton(17,11,"Isolated ",0,componente[8]);
  //}
  //else
  //{
  // componente[8]=newtRadiobutton(5,11,"Sharing ",0,NULL);
  // componente[9]=newtRadiobutton(17,11,"Isolated ",1,componente[8]);
  //}
  if(strcasecmp(tempgrp->borrow,"yes")==0)
  {
   componente[8]=newtRadiobutton(5,11,"Borrow  ",1,NULL);
   componente[9]=newtRadiobutton(17,11,"Bounded  ",0,componente[8]);
  }
  else
  {
   componente[8]=newtRadiobutton(5,11,"Borrow  ",0,NULL);
   componente[9]=newtRadiobutton(17,11,"Bounded  ",1,componente[8]);
  }
  componente[10]=newtButton(22,15,"Done");
  componente[11]=newtButton(32,15,"Cancel");
 
  i=0;
  for (i=0;i<12;i++)
    newtFormAddComponent(formulario,componente[i]);


   newtFormRun(formulario,&salida);
   
   if(salida.u.co==componente[10] || salida.reason==0)
    {
    changeH(tempgrp->name,newtEntryGetValue(componente[5]),ptg,ptc);
    strcpy(tempgrp->name,newtEntryGetValue(componente[5]));
    strcpy(tempgrp->parent,newtEntryGetValue(componente[6]));
    strcpy(tempgrp->rate,newtEntryGetValue(componente[7]));
    //    if(newtRadioGetCurrent(componente[8])==componente[8])
    //  strcpy(tempgrp->share,"yes");
    //else
    //  strcpy(tempgrp->share,"no");
    if(newtRadioGetCurrent(componente[8])==componente[8])
      strcpy(tempgrp->borrow,"yes");  
    else
      strcpy(tempgrp->borrow,"no");  
   }  
   newtFormDestroy(formulario);
   newtPopWindow();
}

void delH(char *name,struct grupos **ptg,struct clientes **ptc)
{

  struct grupos *tmpgrp,*nxtg;
  struct clientes *tmpcli,*tmpc;
  
   tmpgrp=*ptg;
	      while(tmpgrp!=NULL)
	      {
	       nxtg=tmpgrp->nxt;
	       if((strcasecmp(tmpgrp->parent,name)==0))
	           delG(tmpgrp,ptg,ptc);
	       tmpgrp=nxtg;
	      }
   tmpcli=*ptc;
	    while(tmpcli!=NULL)
	      {
	       tmpc=tmpcli->nxt;
	       if(strcasecmp(tmpcli->parent,name)==0)
		 delC(tmpcli,ptc);
	       tmpcli=tmpc;
	      }
}
void delG(struct grupos *grp,struct grupos **ptg,struct clientes **ptc)
{
 struct grupos *tempgrp;
	  tempgrp=*ptg;
	  
	 if(tempgrp==grp)
	   *ptg=tempgrp->nxt;
	      
	    while(tempgrp!=NULL)
	    {
	       if(tempgrp->nxt==grp)
		 tempgrp->nxt=(tempgrp->nxt)->nxt;
	       else
		 tempgrp=tempgrp->nxt;
	    }
}

void delC(struct clientes *cli,struct clientes **ptc)
{
 struct clientes *tempcli;
 
         tempcli=*ptc;

	 if(tempcli==cli)
	      *ptc=tempcli->nxt;
	      
	    while(tempcli!=NULL)
	    {
	       if(tempcli->nxt==cli)
		 tempcli->nxt=(tempcli->nxt)->nxt;
	       else
		 tempcli=tempcli->nxt;
	    }
}
void AddMC(struct clientes **ptc)//Agregar Varios Clientes Simultaneamente
{
 struct newtExitStruct salida;
 struct clientes *tmpcli;
 const char **field=NULL;
 char namec[20],ipc[25],borrowc[5],ipf[25],ipl[25];
 long int ipf1,ipf2,ipf3,ipf4,ipl1,ipl2,ipl3,ipl4;
 newtComponent formulario,componente[16];
 int i;
 

  newtCenteredWindow(50,21,"Configuracion Clientes");
  formulario=newtForm(NULL,NULL,0);

  componente[0]=newtTextboxReflowed(1,0,"Please set the client configuration\nUse this speeds: kbit,mbit,bps"
  					"\nBorrow is to borrow bandwith",40, 10, 10, 0);
  componente[1]=newtTextboxReflowed(4,5,"Add Multiple Clients",40 , 10, 10, 0);
  componente[2]=newtLabel(5,7,"Name:");
  componente[3]=newtLabel(5,8,"Parent:");
  componente[4]=newtLabel(5,9,"First IP:");
  componente[5]=newtLabel(5,10,"Last  IP:");
  componente[6]=newtLabel(5,11,"Rate:");
  componente[7]=newtEntry(14,7,NULL,15,field,0);
  componente[8]=newtEntry(14,8,grupoactual,15,field,NEWT_FLAG_DISABLED);
  componente[9]=newtEntry(14,9,NULL,15,field,0);
  componente[10]=newtEntry(14,10,NULL,15,field,0);
  componente[11]=newtEntry(14,11,NULL,15,field,0);
  componente[12]=newtRadiobutton(5,13,"Borrow  ",1,NULL);
  componente[13]=newtRadiobutton(17,13,"Bounded  ",0,componente[12]);
  componente[14]=newtButton(22,16,"Done");
  componente[15]=newtButton(32,16,"Cancel");  

  i=0;
  for (i=0;i<16;i++)
    newtFormAddComponent(formulario,componente[i]);


  newtFormRun(formulario,&salida);
  
  if(salida.u.co==componente[14] || salida.reason==0)
   {
    
    	
    strncpy(ipf,newtEntryGetValue(componente[9]),25);
    strncpy(ipl,newtEntryGetValue(componente[10]),25);

    ipf1=strtol(ipf,NULL,0);
    ipf2=strtol(strchr(ipf,'.')+1,NULL,0);
    ipf3=strtol(strchr(strchr(ipf,'.')+1,'.')+1,NULL,0);
    ipf4=strtol(strchr(strchr(strchr(ipf,'.')+1,'.')+1,'.')+1,NULL,0);

    ipl1=strtol(ipl,NULL,0);
    ipl2=strtol(strchr(ipl,'.')+1,NULL,0);
    ipl3=strtol(strchr(strchr(ipl,'.')+1,'.')+1,NULL,0);
    ipl4=strtol(strchr(strchr(strchr(ipl,'.')+1,'.')+1,'.')+1,NULL,0);
    
    if(ipf1==ipl1 && ipf2==ipl2 && ipf3==ipl3)
     {
	tmpcli=*ptc;

	//Si es el primer cliente lo creo si no busco el ultimo

      if(tmpcli==NULL)
	{
	  *ptc=(struct clientes *)malloc(sizeof(struct clientes)); 
	  tmpcli=*ptc;
	}
	else
	{	
	 while(tmpcli->nxt!=NULL)
           tmpcli=tmpcli->nxt;
	}
       tmpcli->nxt=NULL;
	
        if(newtRadioGetCurrent(componente[12])==componente[12])
             strcpy(borrowc,"yes");  
           else
            strcpy(borrowc,"no");
	
	for(i=ipf4;i<=ipl4;i++)
	  {
	    tmpcli->nxt=(struct clientes *)malloc(sizeof(struct clientes));
            tmpcli=tmpcli->nxt;
	    
	    tmpcli->nxt=NULL;

	    sprintf(namec,"%s.%d",newtEntryGetValue(componente[7]),i);
	    strcpy(tmpcli->name,namec);
	    sprintf(ipc,"%d.%d.%d.%d",(int)ipf1,(int)ipf2,(int)ipf3,i);
            strcpy(tmpcli->ip,ipc);
	    strcpy(tmpcli->parent,newtEntryGetValue(componente[8]));
	    strcpy(tmpcli->borrow,borrowc);
            strcpy(tmpcli->rate,newtEntryGetValue(componente[11]));
	    
	  }
      }
    }
}


void AddC(struct clientes **ptc) //Agregar Clientes
{
 struct newtExitStruct salida;
 struct clientes *tmpcli;
 const char **field=NULL;
 newtComponent formulario,componente[16];
 int i;

  newtCenteredWindow(50,21,"Configuracion Clientes");
  formulario=newtForm(NULL,NULL,0);

  componente[0]=newtTextboxReflowed(1,0,"Please set the client configuration\nUse this speeds: kbit,mbit,bps"
  					"\nBorrow is to borrow bandwith",40, 10, 10, 0);
  componente[1]=newtTextboxReflowed(4,5,"Add Client",40 , 10, 10, 0);
  componente[2]=newtLabel(5,7,"Name:");
  componente[3]=newtLabel(5,8,"Parent:");
  componente[4]=newtLabel(5,9,"Rate");
  componente[5]=newtLabel(5,10,"IP:");
  componente[6]=newtEntry(14,7,NULL,15,field,0);
  componente[7]=newtEntry(14,8,grupoactual,15,field,NEWT_FLAG_DISABLED);
  componente[8]=newtEntry(14,9,NULL,15,field,0);
  componente[9]=newtEntry(14,10,NULL,20,field,0);
  //componente[10]=newtRadiobutton(5,12,"Sharing ",1,NULL);
  //componente[11]=newtRadiobutton(17,12,"Isolated ",0,componente[10]);
  componente[10]=newtRadiobutton(5,12,"Borrow  ",1,NULL);
  componente[11]=newtRadiobutton(17,12,"Bounded  ",0,componente[10]);
  componente[12]=newtButton(22,16,"Done");
  componente[13]=newtButton(32,16,"Cancel");  

  i=0;
  for (i=0;i<14;i++)
    newtFormAddComponent(formulario,componente[i]);


  newtFormRun(formulario,&salida);
  
  if(salida.u.co==componente[12] || salida.reason==0)
   {
    tmpcli=*ptc;
    if(tmpcli==NULL)
	{
	  *ptc=(struct clientes *)malloc(sizeof(struct clientes)); 
	  tmpcli=*ptc;
	}
	else
	{	
	 while(tmpcli->nxt!=NULL)
           tmpcli=tmpcli->nxt;
	   
	 tmpcli->nxt=(struct clientes *)malloc(sizeof(struct clientes));
         tmpcli=tmpcli->nxt;
	}
    tmpcli->nxt=NULL;
    	
    strcpy(tmpcli->name,newtEntryGetValue(componente[6]));
    strcpy(tmpcli->parent,newtEntryGetValue(componente[7]));
    strcpy(tmpcli->rate,newtEntryGetValue(componente[8]));
    strcpy(tmpcli->ip,newtEntryGetValue(componente[9]));
    //if(newtRadioGetCurrent(componente[10])==componente[10])
    //  strcpy(tmpcli->share,"yes");
    //else
    //  strcpy(tmpcli->share,"no");
    if(newtRadioGetCurrent(componente[10])==componente[10])
      strcpy(tmpcli->borrow,"yes");  
    else
      strcpy(tmpcli->borrow,"no");  
   }
  
  newtFormDestroy(formulario);
  newtPopWindow();

}

void AddG(struct grupos **ptg)//Agregar Grupo
{

 struct newtExitStruct salida;
 struct grupos *tempgrp;
 const char **field=NULL;
 newtComponent formulario,componente[14];
 int i;
  
  newtCenteredWindow(50,20,"Configuracion Grupos");
  formulario=newtForm(NULL,NULL,0);

  componente[0]=newtTextboxReflowed(1,0,"Please set the group configuration\nUse this speeds: kbit,mbit,bps"
  					"\nBorrow is to borrow bandwith",40, 10, 10, 0);
  componente[1]=newtTextboxReflowed(4,5,"Add Group",40 , 10, 10, 0);
  componente[2]=newtLabel(5,7,"Name:");
  componente[3]=newtLabel(5,8,"Parent:");
  componente[4]=newtLabel(5,9,"Rate");
  componente[5]=newtEntry(14,7,NULL,15,field,0);  
  componente[6]=newtEntry(14,8,grupoactual,15,field,NEWT_FLAG_DISABLED);
  componente[7]=newtEntry(14,9,NULL,15,field,0);
  //componente[8]=newtRadiobutton(5,11,"Sharing ",1,NULL);
  //componente[9]=newtRadiobutton(17,11,"Isolated ",0,componente[8]);
  componente[8]=newtRadiobutton(5,11,"Borrow  ",1,NULL);
  componente[9]=newtRadiobutton(17,11,"Bounded  ",0,componente[8]);
  componente[10]=newtButton(22,15,"Done");
  componente[11]=newtButton(32,15,"Cancel");
 
  i=0;
  for (i=0;i<12;i++)
    newtFormAddComponent(formulario,componente[i]);


   newtFormRun(formulario,&salida);
   
   if(salida.u.co==componente[10] || salida.reason==0)
    {
    tempgrp=*ptg;
    if(tempgrp==NULL)
	{
	  *ptg=(struct grupos *)malloc(sizeof(struct grupos)); 
	  tempgrp=*ptg;
	}
	else
	{	
	 while(tempgrp->nxt!=NULL)
           tempgrp=tempgrp->nxt;
	   
	 tempgrp->nxt=(struct grupos *)malloc(sizeof(struct grupos));
         tempgrp=tempgrp->nxt;
	}
    tempgrp->nxt=NULL;
    strcpy(tempgrp->name,newtEntryGetValue(componente[5]));
    strcpy(tempgrp->parent,newtEntryGetValue(componente[6]));
    strcpy(tempgrp->rate,newtEntryGetValue(componente[7]));
    //    if(newtRadioGetCurrent(componente[8])==componente[8])
    //  strcpy(tempgrp->share,"yes");
    //else
    //  strcpy(tempgrp->share,"no");
    if(newtRadioGetCurrent(componente[8])==componente[8])
      strcpy(tempgrp->borrow,"yes");  
    else
      strcpy(tempgrp->borrow,"no");  
   }  
   newtFormDestroy(formulario);
   newtPopWindow();
}

void loadlistbox(newtComponent texto,newtComponent lista,struct root *ptroot,struct grupos *ptgrp,struct clientes *ptcli,char *grupo)
{
 char txt[60];
 struct grupos *tempgrp;
 struct clientes *tempcli;
 int wsize;
  

  if((strcasecmp(grupo,"root"))==0)
  {
    sprintf(txt,"Group: %s---->%s",grupo,ptroot->linkspeed);
  }
  else //Busco la velocidad del grupo para el titulo
  {
   tempgrp=ptgrp;
   while(tempgrp!=NULL)
   {
    if(strcasecmp(grupo,tempgrp->name)==0)
    {
     sprintf(txt,"Group: %s---->%s",grupo,tempgrp->rate);
     break;
    }
   else
    {
     sprintf(txt,"Group: %s---->Error!!!",grupo);
     tempgrp=tempgrp->nxt;
    }
   }
  }
  
 strcpy(grupoactual,grupo);
 newtTextboxSetText(texto,txt);
  

 //Agrego los grupos
 
 wsize=20;//Ancho por defecto
 
 newtListboxClear(lista);
 newtListboxSetWidth(lista,wsize);
 
  tempgrp=ptgrp;//Para volver a empezar
  tempcli=ptcli;
 
  while(tempgrp!=NULL)
  {
   if(strcasecmp(grupo,tempgrp->parent)==0)
    {
     sprintf(txt,"G %s-->%s ",tempgrp->name,tempgrp->rate);
     if(wsize<strlen(&txt[0])+10)
        { 
	 wsize=strlen(&txt[0])+10;
	 newtListboxSetWidth(lista,wsize);
	}
	
     newtListboxAppendEntry(lista,txt,(void *)tempgrp);
     tempgrp=tempgrp->nxt;
    }
   else
    tempgrp=tempgrp->nxt;
  }
    
 //Agrego los clientes

   while(tempcli!=NULL)
    {
     if(strcasecmp(grupo,tempcli->parent)==0)
      {
       sprintf(txt,"c %s->%s->%s ",tempcli->name,tempcli->rate,strrchr(tempcli->ip,'.'));    
       if(wsize<strlen(&txt[0])+10)
        { 
	 wsize=strlen(&txt[0])+10;
	 newtListboxSetWidth(lista,wsize);
	}
	
       newtListboxAppendEntry(lista,txt,(void *)tempcli);
       tempcli=tempcli->nxt;
      }
     else
      tempcli=tempcli->nxt;
    }
}

int ClassConfig(struct root **ptr,struct grupos **ptg,struct clientes **ptc)//Pantalla Principal
{
  newtComponent formulario,texto,lista,add,del,change,backlist,Done,Cancel,Back;
  int x,y,rst,resultado;
  struct newtExitStruct salida;
  void *current;
  struct clientes *tempcli;
  struct grupos *tempgrp;
 
  newtCenteredWindow(70,20,"Configuration Groups/Clients");
  formulario=newtForm(NULL,NULL,0);
  texto=newtTextbox(1,1,60,1,NEWT_FLAG_WRAP);
  lista=newtListbox(10,3,8,NEWT_FLAG_SCROLL | NEWT_FLAG_BORDER | NEWT_FLAG_RETURNEXIT);
  
  loadlistbox(texto,lista,*ptr,*ptg,*ptc,"root");
  
  x=12;
  y=12;
  
  add=newtCompactButton(x,y,"add");
  del=newtCompactButton(x+7,y,"del");
  change=newtCompactButton(x+14,y,"change");
  backlist=newtCompactButton(x+24,y,"back");
  Done=newtButton(x-3,y+3,"Done");
  Cancel=newtButton(x+7,y+3,"Cancel");
  Back=newtButton(x+19,y+3,"Back");
    
  newtFormAddComponents(formulario,texto,lista,add,del,change,backlist,Done,Cancel,Back,NULL);
    
  do
  {
   newtFormRun(formulario,&salida);
   
   if	  (salida.u.co==add)
           {
	    rst=newtWinTernary("Configuration Groups/Clients","Group","Client","Cancel","What do you want to add?");
	     if(rst==1)
	       AddG(ptg);
	     if(rst==2)
	      {
	       rst=newtWinTernary("Configuration Clients","Client","Multiple","Cancel","What do you want to add?");
	        if(rst==2)
	       	 AddMC(ptc);
	        if(rst==1)
	         AddC(ptc);
	      }

	   
           loadlistbox(texto,lista,*ptr,*ptg,*ptc,grupoactual);
	   }
   else if(salida.u.co==del)
   	   {
   	    current=newtListboxGetCurrent(lista);
            
	    tempcli=*ptc;
	    tempgrp=*ptg;
	    
	    while(tempcli!=NULL)
	    {
	     if(tempcli==(struct clientes *)current)
	      {
               rst=newtWinChoice("Configuration Clients","Yes","No"," Delete the Client?");
 		if (rst==0)//Si apreto F12
		  rst=1;
		if (rst==1)
		  delC(tempcli,ptc);
	       break;
	      }
	     else
	       tempcli=tempcli->nxt;
	    }
	    
	    while(tempgrp!=NULL)
	    {
	     if(tempgrp==(struct grupos *)current)
	      {
		rst=newtWinChoice("Configuration Groups","Yes","No"," Delete the Group?");
		 if (rst==0)//Si apreto F12
		  rst=1;
		if (rst==1)
		{
	         delH(tempgrp->name,ptg,ptc);
                 delG(tempgrp,ptg,ptc);
		}
	        break;
	      }
	     else
	       tempgrp=tempgrp->nxt;
	    }
	    loadlistbox(texto,lista,*ptr,*ptg,*ptc,grupoactual);	    
	   }
	   
   else if(salida.u.co==change)
         {
           current=newtListboxGetCurrent(lista);

	    tempcli=*ptc;
	    tempgrp=*ptg;
	    

	    while(tempcli!=NULL)
	    {
	     if(tempcli==(struct clientes *)current)
	      {
       	   
	       changeC(tempcli);
	       break;
	       
	      }
	     else
	       tempcli=tempcli->nxt;
	    }
	    
	    while(tempgrp!=NULL)
	    {
	     if(tempgrp==(struct grupos *)current)
	      {
		changeG(tempgrp,ptg,ptc);
	        break;
	      }
	      
	     else
	       tempgrp=tempgrp->nxt;
	    }
	    loadlistbox(texto,lista,*ptr,*ptg,*ptc,grupoactual);
	  } 
	
   else if(salida.u.co==lista)
   	   {
	    current=newtListboxGetCurrent(salida.u.co);
            tempcli=*ptc;
	    tempgrp=*ptg;
	    while(tempgrp!=NULL)
	    {
	     if(tempgrp==(struct grupos *)current)
	      break;
	     else
	       tempgrp=tempgrp->nxt;
	    }
    	   if (tempgrp!=NULL)
   	   	loadlistbox(texto,lista,*ptr,*ptg,*ptc,tempgrp->name);
	   }
   else if(salida.u.co==backlist)
   	loadlistbox(texto,lista,*ptr,*ptg,*ptc,getparent(grupoactual,*ptg));
  
  }while(salida.u.co!=Cancel && salida.u.co!=Done && salida.u.co!=Back);
  

  if(salida.u.co==Cancel)
  {
    rst=newtWinChoice("Configuraction Traffic Control","Yes","No"," Do you want to Exit?");
    if (rst==0)//Si apreto F12
      rst=1;
    if(rst==1)
     {
      resultado=-1;
     }
     else
     {
      resultado=1;
     }
   }
  	
  if(salida.u.co==Back)
  	resultado=1;
  if(salida.u.co==Done)
   {
    rst=newtWinChoice("Configuration Traffic Control","Yes","No","Save Changes?");
    if (rst==0)//Si apreto F12
      rst=1;
    if(rst==1)
     {
      if(writeconf(*ptr,*ptg,*ptc)==-1){  //Si quiero guardar la configuracion llamo a writeconf
	newtWinMessage("Error!!!","OK","Error en /etc/polip.conf\nYou must be root");
	resultado=-1;
      }
      else
	resultado=0;
	     
     }
     else
     {
      resultado=0;
     }
   }
	
  newtFormDestroy(formulario);
  newtPopWindow();
  
  return resultado;
  
}//Fin ClassConfig

int NetConfig(struct root **ptr,struct grupos **ptg,struct clientes **ptc)//Pantalla inicial
{
  newtComponent formulario,componente[12];
  int i,resultado;
  struct newtExitStruct salida;
  const char **ptchar=NULL;
  struct root *ptroot;
  
   if(*ptr==NULL)
    *ptr=(struct root *)calloc(1,sizeof(struct root));
   
   ptroot=*ptr;
      
  newtCenteredWindow(70,20,"Devices Config");
  formulario=newtForm(NULL,NULL,0);
  
  char *title="Please set up the devices\nDevices: eth0,eth1,ppp0,tr0\nSpeed: kbit,mbit,bps (no blanks)";
  componente[0]=newtTextboxReflowed(1,1,title, 60, 10, 10, 0);
  componente[1]=newtTextboxReflowed(3,5,"Internet Device", 60, 10, 10, 0);
  componente[2]=newtLabel(5,7,"Device:");
  componente[3]=newtLabel(5,8,"Speed:");
  componente[4]=newtTextboxReflowed(3,10,"Local Network Device", 60, 10, 10, 0);
  componente[5]=newtLabel(5,12,"Device:");
  componente[6]=newtLabel(5,13,"Speed:");
  componente[7]=newtEntry(14,7,ptroot->linkdevice,15,ptchar,0);  
  componente[8]=newtEntry(14,8,ptroot->linkspeed,15,ptchar,0);
  componente[9]=newtEntry(14,12,ptroot->netdevice,15,ptchar,0);
  componente[10]=newtEntry(14,13,ptroot->netspeed,15,ptchar,0);
  componente[11]=newtButton(40,15,"Next");
  componente[12]=newtButton(50,15,"Cancel");
  
  i=0;
  for (i=0;i<13;i++)
    newtFormAddComponent(formulario,componente[i]);


   newtFormRun(formulario,&salida);
    
  if(salida.u.co==componente[11] || salida.reason==0)
   {
    strcpy(ptroot->linkdevice,newtEntryGetValue(componente[7]));
    strcpy(ptroot->linkspeed,newtEntryGetValue(componente[8]));
    strcpy(ptroot->netdevice,newtEntryGetValue(componente[9]));
    strcpy(ptroot->netspeed,newtEntryGetValue(componente[10]));
    resultado=ClassConfig(ptr,ptg,ptc);
   }
 else
    resultado=-1;
    
  newtFormDestroy(formulario);
  newtPopWindow();
  return resultado;
}



int main(void)
{
 
 int rst;
 struct root **ptr,*ptroot=NULL;
 struct grupos **ptg,*ptgrp=NULL;
 struct clientes **ptc,*ptcli=NULL;
   
 ptr=&ptroot;
 ptg=&ptgrp;
 ptc=&ptcli; 
 
 if(readconf(ptr,ptg,ptc)!=0){
	  fprintf(stderr,"Polipd Error: /etc/polip.conf");
	  exit(0);
 }	
 
 newtInit();
 newtCls();
 newtPushHelpLine(" <Tab>/<Alt-Tab> change   |   <Space> select  |   <F12> next");
 newtDrawRootText(0, 0, "Polip Internet Traffic Control");

 rst=newtWinChoice("Configuration Traffic Control","Yes","No"," Config Polip?");
 
 if (rst==0)//Si apreto F12
  rst=1;
 

 if (rst==1)
 {
   do
   {
    rst=NetConfig(ptr,ptg,ptc);
   }while(rst!=0 && rst!=-1);
   
 }//If mayor de si=1
 
 newtFinished(); 
 
 
 return 0;
}
