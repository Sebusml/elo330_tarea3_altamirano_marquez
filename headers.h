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
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h> 
#include <poll.h>
#include <fcntl.h>

#define BUFSIZE 1024*64


long int retardo_promedio;
long int variacion_retardo;
long int porcentaje_perdida;

/*Estructuras para sockets*/
struct sockaddr_in sockaddr_client; 
struct sockaddr_in temp_sockaddr_client; 
struct sockaddr_in sockaddr_server; 
struct sockaddr_in temp_sockaddr_server; 
int client_s; 
int server_s; 

/*Candados y variables de condicion para hebras*/
pthread_mutex_t		 lock_buff_cts = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t 	 lock_buff_stc = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t     cond_buff_cts = PTHREAD_COND_INITIALIZER;
pthread_cond_t     cond_buff_stc = PTHREAD_COND_INITIALIZER;

/*Descriptores de archivos para pipas*/
int buff_data_cs[2];
int buff_data_sc[2]; 
int buff_time_cs[2];
int buff_time_sc[2];
 
/*Funciones de inicializacion y varias*/
void init(int argc,char *argv[]);
void open_sockets(void);
void thread_start(pthread_t *);
void open_pipes(void);
long int sim_delay (struct timespec *,long int * );
long int calc_max(long int, long int );
long int rand_range( long int, long int );

/*Threads functions*/
void * listen_client_t(void *arg);
void * listen_server_t(void *arg);
void * send_client_t(void *arg);
void * send_server_t(void *arg);


