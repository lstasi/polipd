#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <popt.h>


 //Estructura de configuracion de los grupos
struct grupos
{
 char name[20],parent[20],rate[20], borrow[5], share[5];
 int handle;
 struct grupos *nxt;
};

//Estructura de configuracion para los clientes
struct clientes
{
 char name[20],parent[20],rate[20], borrow[5], share[5], ip[25]; 
 int handle;
 int speed; 
 pid_t pstats;
 time_t tstats;
 struct clientes *nxt;
};
//Estructura root
struct root
{
 char netdevice[20],linkdevice[20],netspeed[20], linkspeed[20];
 int handle;
};

//Prototypes 
 
char * itoa(int i);
char * cnl(char *prm);
int rate(char *utins);
char * getipadress(char *ifname);

int setup (struct root *ptrroot,struct grupos *ptrgrp,struct clientes *ptrcli);
int server(struct root *ptrroot,struct grupos *ptrgrp,struct clientes *ptrcli);
int readconf (struct root **ptr,struct grupos **ptg,struct clientes **ptc);
int writeconf (struct root *ptr,struct grupos *ptg,struct clientes *ptc);
void stats(struct clientes *ptcli,struct root *ptrroot);
