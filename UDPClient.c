/*
NAME:       UDP.ECHO.CLIENT
SYNOPSIS:    UDPClient hostid portnumber 
DESCRIPTION:    The program creates a datagram socket in the inet 
                domain, get messsages  typed by a user
                and send them to echoserver running on hostid
                Then it waits for a reply from  the echoserver
                and show it back to the user, with a message
                indicating if there is an error during the round trip. 
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#define MAXHOSTNAME 80
#define BUFSIZE 1024*64
char buf[BUFSIZE];
char rbuf[BUFSIZE];

main( argc, argv )
int argc;
char *argv[];
{
   int   sd;
   struct   sockaddr_in server;
   struct  hostent *hp, *gethostbyname();
   struct  servent *sp;
   struct sockaddr_in from;
   struct sockaddr_in addr;
   int fromlen;
   int length;
   int rc, cc;
   char ThisHost[80];
   int one=1;
   
   sp = getservbyname("echo", "udp");
/* get EchoClient Host information, NAME and INET ADDRESS */

   gethostname(ThisHost, MAXHOSTNAME);

/* OR strcpy(ThisHost,"localhost"); */


   printf("----EchoCleint running at host NAME: %s\n", ThisHost);
   if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
      fprintf(stderr, "Can't find host %s\n", ThisHost);
      exit(-1);
   }
   bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
   printf("    (EchoCleint INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

/* get EchoServer Host information, NAME and INET ADDRESS */

   if  ( (hp = gethostbyname(argv[1])) == NULL ) {
      addr.sin_addr.s_addr = inet_addr(argv[1]);
      if ((hp = gethostbyaddr((char*)&addr.sin_addr.s_addr,
          sizeof(addr.sin_addr.s_addr),AF_INET)) == NULL) {
         fprintf(stderr, "Can't find host %s\n", argv[1]);
         exit(-1);
      }
   }
   printf("----EchoServer running at host NAME: %s\n", hp->h_name);
   bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
   printf("    (EchoServer INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

/* Construct name of socket to send to. */
   server.sin_family = hp->h_addrtype;
   /* server.sin_family = AF_INET; */

   server.sin_port = htons(atoi(argv[2]));
   /*OR    server.sin_port = sp->s_port; */

   /*   Create socket on which to send  and receive */

   sd = socket (hp->h_addrtype,SOCK_DGRAM,0);
   /*OR  sd = socket (PF_INET,SOCK_DGRAM,0); */

   if (sd<0) {
      perror("opening datagram socket");
      exit(-1);
   }
/* get data from USER, send it SERVER,
      receive it from SERVER, display it back to USER  */
   /*   setsockopt(sd, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one));*/
   
   for(;;) {
      printf("\nType anything followed by RETURN, or type CTRL-D to exit\n");
      cleanup(buf);
      cleanup(rbuf);
      rc=read(0,buf, sizeof(buf)-1);
      buf[rc]='\0';
      if (rc == 0) break;

      if (sendto(sd, (void *) buf, /*sizeof(buf)*/ ++rc, 0, (struct sockaddr *)&server, sizeof(server)) <0 )
         perror("sending datagram message");
      fromlen= sizeof(from);
      printf("Enviado otro string\n");
      
      
      if ((cc=recvfrom(sd, (void *)rbuf, sizeof(rbuf), 0, (struct sockaddr *) &from, &fromlen)) < 0)
         perror("receiving echo");
      if (cc == rc)
         if (strcmp(buf, rbuf) == 0){
            printf("Echo is good\n");
            printf("  Received: \"%s\"\n", rbuf);
            printf("  from %s:%d\n", inet_ntoa(from.sin_addr),
                ntohs(from.sin_port));
            if ((hp = gethostbyaddr((char*)&from.sin_addr.s_addr,
                sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
               fprintf(stderr, "Can't find host %s\n", 
                   inet_ntoa(from.sin_addr));
            else
               printf("  (Name is : %s)\n", hp->h_name);
         }
         else
            printf("Echo bad -- strings unequal: \"%s\"\n", rbuf);
      else
         printf("Echo bad -- lengths unequal: \"%s\"\n", rbuf);
   }
   printf ("EOF... exit\n");
   close(sd);
   exit (0);
}
cleanup(buf)
char *buf;
{
   int i;
   for(i=0; i<BUFSIZE; i++) buf[i]='\0';
}
