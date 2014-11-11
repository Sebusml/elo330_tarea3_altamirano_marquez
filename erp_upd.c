#include "headers.h"

int main (int argc,char *argv[] )
{
  pthread_t id_t[4];
  
  /*Resolver parametros de entrada*/
  init(argc, argv);
  
  /*Abrir sockets*/
  open_sockets();
  
  /*Crear pipas que actuan como buffer FIFO*/
  open_pipes();
  
  /*Crear hebras*/
  thread_start(id_t);
  /*Hebra principal a dormir*/
  pause();
	exit(0);
}

void * listen_client_t(void *arg)
{
	struct timespec t_arrive;  //Para guardar tiempo actual
	char temp_buff[BUFSIZE];   //Buffer temporal para recivir datos de pipe
	int from_len;							 
	int rc;										 //Guarda cantidad de bytes del datagrama
	from_len = sizeof(temp_sockaddr_client);
	
	/*Para comprobar si hay datos nuevos en el descriptor de archivo*/
	struct pollfd new_data;    
	new_data.fd = client_s;
	new_data.events = POLLIN;
	
	while(1)
	{
		if (poll(&new_data, 1, 100)) // Comprueba si hay datos nuevos en el descriptor de archivo del socket
		{
		rc=recvfrom(client_s, temp_buff, sizeof(temp_buff), 0, (struct sockaddr *)&temp_sockaddr_client, &from_len);
		pthread_mutex_lock(&lock_buff_cts);
		
		clock_gettime(CLOCK_REALTIME, &t_arrive);
 		printf("Datos recividos desde cliente %s",temp_buff);
		write(buff_data_cs[1], &temp_buff, rc );											// Copia datagrama en buffer FIFO
		write(buff_time_cs[1], &t_arrive, sizeof(struct timespec) );	// Copia tiempo de arrivo paquete en buffer FIFO
		
		pthread_mutex_unlock(&lock_buff_cts);
		pthread_cond_signal(&cond_buff_cts);
		}

	}
	
	return((void *)0);
}

void * send_server_t(void *arg)
{
	struct timespec t_arrive;
	long int t_re_ant = 0;				//Guarda tiempo envio paquete anterior
	char temp_buff[BUFSIZE];
	int  size_data;
	while(1)
	{
		pthread_cond_wait(&cond_buff_cts, &lock_buff_cts);
		size_data = read(buff_data_cs[0], &temp_buff, sizeof( temp_buff));
		read(buff_time_cs[0], &t_arrive,sizeof(struct timespec) );
		
		
		
		/*Simulacion de retardo*/
		usleep(sim_delay( &t_arrive, &t_re_ant));
		if ( rand_range(0,100) > porcentaje_perdida ) 
		{
			printf("Enviando Datos a servidor: %s", temp_buff);
			if (sendto(server_s, temp_buff, size_data, 0, (struct sockaddr *)&sockaddr_server, sizeof(sockaddr_server)) < 0 )
			  perror("sending datagram message");
		}
		else bzero(temp_buff,sizeof(temp_buff));
		pthread_mutex_unlock(&lock_buff_cts);
	}	
	return((void *)0);
}

void * listen_server_t(void *arg)
{
	struct timespec t_arrive;
	char temp_buff[BUFSIZE];
	int from_len;
	int rc;
	from_len = sizeof(temp_sockaddr_client);
	struct pollfd new_data;
	new_data.fd = server_s;
	new_data.events = POLLIN;
	
	while(1){
	  if (poll(&new_data,1,100))
	  {
	    if ((rc=recvfrom(server_s, temp_buff, sizeof(temp_buff), 0, (struct sockaddr *) &temp_sockaddr_server, &from_len)) < 0)
	      perror("receiving echo");
	    pthread_mutex_lock(&lock_buff_stc);
	    
	    clock_gettime(CLOCK_REALTIME, &t_arrive);
	    printf("Datos recibidos desde servidor: %s",temp_buff );
	    write(buff_data_sc[1], &temp_buff, rc );
	    write(buff_time_sc[1], &t_arrive, sizeof(struct timespec) );	
	    
	    pthread_mutex_unlock(&lock_buff_stc);
	    pthread_cond_signal(&cond_buff_stc);

	  }
  }
  return((void *)0);
}

void * send_client_t(void *arg)
{
	long int t_re_ant = 0;
	struct timespec t_arrive;
	char temp_buff[BUFSIZE];
	int  size_data;

	while(1)
	{
	  pthread_cond_wait(&cond_buff_stc, &lock_buff_stc);
	  
	  read(buff_time_sc[0], &t_arrive, sizeof(struct timespec) );
	  size_data = read(buff_data_sc[0], &temp_buff, sizeof( temp_buff));
	  
	  usleep(sim_delay( &t_arrive, &t_re_ant));
	  
	  if ( rand_range(0,100) > porcentaje_perdida ) 
	  {
	  	printf("Enviando datos al cliente: %s \n", temp_buff);
	  	if (sendto(client_s, temp_buff, size_data, 0, (struct sockaddr *)&temp_sockaddr_client, sizeof(sockaddr_client)) < 0 )
	  	  perror("sending datagram message to client");
	  }
	  else bzero(temp_buff,sizeof(temp_buff));
	  
	  pthread_mutex_unlock(&lock_buff_stc);
	}
	return((void *)0);
}

void thread_start(pthread_t * id_t)
{
  int i;
  int err[4];
  err[0] = pthread_create(&id_t[0], NULL, listen_client_t, NULL);
  err[1] = pthread_create(&id_t[1], NULL, send_server_t, NULL);
  err[2] = pthread_create(&id_t[2], NULL, listen_server_t, NULL);
  err[3] = pthread_create(&id_t[3], NULL, send_client_t, NULL);
  for (i=0; i<4;i++)
  	{
  		if (err[i] != 0)
  			{
					printf("Error en crear hebra \n");  				
  				exit(-1);
  			}
  	}
}

void open_sockets(void)
{
  /*Sockets para cliente y servidor*/
  client_s = socket(AF_INET, SOCK_DGRAM, 0);
  server_s = socket(AF_INET, SOCK_DGRAM, 0);
  
  /*bind hacia el cliente*/
  if ( bind(client_s, (struct sockaddr *) &sockaddr_client, sizeof(struct sockaddr_in))){
    close(client_s);
    perror("binding name to datagram socket");
    exit(-1);
    }
}	

void open_pipes()
{
	pipe(buff_data_cs);
	pipe(buff_data_sc);
	pipe(buff_time_cs);
	pipe(buff_time_sc);
}

void init ( int argc, char *argv[] )
{
	switch (argc)
	{
		case 6: 
			sockaddr_client.sin_family = AF_INET;
			sockaddr_client.sin_port = htons( strtol(argv[4],NULL,10) );
			sockaddr_client.sin_addr.s_addr = htonl(INADDR_ANY); 
			sockaddr_server.sin_family = AF_INET;
			sockaddr_server.sin_port = htons( strtol(argv[5],NULL,10) );
			sockaddr_server.sin_addr.s_addr = inet_addr("127.0.0.1"); 	
			break;
		case 7:
			sockaddr_client.sin_family = AF_INET;
			sockaddr_client.sin_port = htons( strtol(argv[4],NULL,10) );
			sockaddr_client.sin_addr.s_addr = htonl(INADDR_ANY); 
			sockaddr_server.sin_family = AF_INET;
			sockaddr_server.sin_port = htons( strtol(argv[6],NULL,10) );
			sockaddr_server.sin_addr.s_addr = inet_addr(argv[5]); 	
			break;
		default:		
			printf("Cantidad de argumentos invalidos \n");
			exit(-1);
	}
	retardo_promedio   = strtol(argv[1],NULL,10);
	variacion_retardo  = strtol(argv[2],NULL,10);
	porcentaje_perdida = strtol(argv[3],NULL,10);
}

long int sim_delay (struct timespec *t_arrive,long int * t_re_ant)
{
  long int t_re;
  long int t_a;
  long int min, max, delay;
	/*Transforma a microsegundos*/
  t_a = t_arrive->tv_sec * 1000000 + t_arrive->tv_nsec/1000;
  if ( *t_re_ant == 0)
  	*t_re_ant = t_a;

  /* Se calcula rango de random*/
  min = calc_max(t_a + retardo_promedio - variacion_retardo, *t_re_ant);
  max = t_a + retardo_promedio + variacion_retardo;
  
  t_re = rand_range(min,max);
  /*Retardo para salida paquete*/
  delay = t_re - t_a;
  *t_re_ant = t_re;
  return delay;
}

long int calc_max(long int a,long int b)
{
	if (a > b) return a; 
	else       return b;
}

long int rand_range(long int down, long int up)
{
  long int max = up - down;
  unsigned long
    // max <= RAND_MAX < ULONG_MAX, so this is okay.
    num_bins = (unsigned long) max + 1,
    num_rand = (unsigned long) RAND_MAX + 1,
    bin_size = num_rand / num_bins,
    defect   = num_rand % num_bins;

  long x;
  // This is carefully written not to overflow
  while (num_rand - defect <= (unsigned long)(x = random()));

  // Truncated division is intentional
  return x/bin_size + down;
}
