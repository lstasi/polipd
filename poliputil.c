#include "polip.h"

char * itoa(int i)
{
 char *c;
 sprintf(c,"%d",i);
 return c;
}


char * cnl (char *prm)
{
 //Delete newline
 *(strchr(prm,'\0')-1)='\0';
 return prm;
}
/*
* rate is for tranform mbit kbit it to bps
*/
int rate(char *units)
{
	char **p,*tail=NULL;
	double bps;	
	
	p=&tail;	
	bps = strtod(units, p);

        
	if (tail!=NULL) 
	 {
		if (strcasecmp(tail,"kbps") == 0)
			bps = (bps*1024)*8;

		else if (strcasecmp(tail,"mbps") == 0)
			bps = (bps*1024*1024)*8;
			
		else if (strcasecmp(tail,"bps") == 0)
			bps = bps*8;

		else if (strcasecmp(tail,"mbit") == 0)
			bps = bps*1024*1024;
			
		else if (strcasecmp(tail,"kbit") == 0)
			bps = bps*1024;
		else
			bps = bps*8;
		
	 } 
        
	return (int) bps;
}

char * getipadress(char *ifname)
{
  
     struct ifreq ifr;
     int fd;
 
     char *ip;
     
     ip=(char *)malloc(20);

     fd = socket(AF_INET,SOCK_DGRAM, 0);
     if (fd >= 0) {
         strcpy(ifr.ifr_name, ifname);
         if (ioctl(fd, SIOCGIFADDR, &ifr) == 0)
	  {
             sprintf(ip,"%d.%d.%d.%d",
	      		(unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[2],
			(unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[3],
			(unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[4],
			(unsigned char)ifr.ifr_ifru.ifru_addr.sa_data[5]);
         }
     }

     return ip;
}
