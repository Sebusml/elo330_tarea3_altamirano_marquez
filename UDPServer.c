/*
NAME:       UDP.ECHO.SERVER
SYNOPSIS:    UDPServer port_number
DESCRIPTION:    The program creates a datagram socket in the inet 
                domain then receive messsages from client process 
                and echo it back to it.
*/

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <stdlib.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#define MAXHOSTNAME 80

void reusePort(int sock);

main( argc, argv )
int argc;
char *argv[];
{
   int   sd;
   struct   sockaddr_in server;
   struct  hostent *hp, *gethostbyname();
   struct  servent *sp;
   struct sockaddr_in from;
   int fromlen;
   int length;
   char buf[1024*64];
   int rc;
   char ThisHost[80];

   sp = getservbyname("echo", "udp");

/* get EchoServer Host information, NAME and INET ADDRESS */

   gethostname(ThisHost, MAXHOSTNAME);

/* OR strcpy(ThisHost,"localhost"); */

   printf("----EchoServer running at host NAME: %s\n", ThisHost);
   if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
      fprintf(stderr, "Can't find host %s\n", ThisHost);
      exit(-1);
   }
   bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
   printf("    (EchoServer INET ADDRESS is: %s )\n", 
                     inet_ntoa(server.sin_addr));


/* Construct name of socket to send to. */
   server.sin_family = hp->h_addrtype;
   /*OR server.sin_family = AF_INET; */


   server.sin_addr.s_addr = htonl(INADDR_ANY);

    /*server.sin_port = htons(0);  */
   /*OR    server.sin_port = sp->s_port; */
     server.sin_port = htons(atoi(argv[1]));

/* Create socket on which to send  and receive */

   /* sd = socket (PF_INET,SOCK_DGRAM,0); */
   sd = socket (hp->h_addrtype,SOCK_DGRAM,0);

   if (sd<0) {
      perror("opening datagram socket");
      exit(-1);
   }
reusePort(sd);
   if ( bind( sd, (struct sockaddr *)&server, sizeof(server) ) ) {
      close(sd);
      perror("binding name to datagram socket");
      exit(-1);
   }

   length = sizeof(server);
   if ( getsockname (sd,(struct sockaddr *) &server,&length) ) {
      perror("getting socket name");
      exit(0);
   }

   printf("Server Port is: %d\n", ntohs(server.sin_port));

/*  get data from  clients and send it back */
   for(;;){
      fromlen = sizeof(from);
      printf("\n...server is waiting...\n");
      if ((rc=recvfrom(sd, buf, sizeof(buf), 0, (struct sockaddr *)&from, &fromlen)) < 0)
         perror("receiving datagram  message");
      if (rc > 0){
         buf[rc]='\0';
         printf("Received: \"%s\"\n", buf);
         printf("From %s:%d\n", inet_ntoa(from.sin_addr),
             ntohs(from.sin_port));
         if ((hp = gethostbyaddr((char *)&from.sin_addr.s_addr,
             sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
            fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
         else
            printf("(Name is : %s)\n", hp->h_name);

         if (sendto(sd, buf, rc, 0,(struct sockaddr *) &from, sizeof(from)) <0 )
            perror("sending datagram message");
      }
   }
}
void reusePort(int s)
{
  int one=1;
  
  if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *) &one,sizeof(one)) == -1 )
  {
    printf("error in setsockopt,SO_REUSEPORT \n");
    exit(-1);
  }
}
