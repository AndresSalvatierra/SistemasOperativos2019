#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <time.h>
#include "funciones.h"

#define TAM 1024

struct satelites	//Informacion referida al satelite
{
	char id[20];
	char uptime[20];	
	char version[20];
	char cpu[20];
	char memoria[20];
};

void info_satelite();
void dif_hora();

static struct satelites satelite;
static struct tm *tlocal;
static int minuto,segundo;

void update(int sockfd, char *argv[]);
void info_satelite();
void telemetria(char *argv[]);
void dif_hora();
void scanning(int sockfd);

int main( int argc, char *argv[] ) {
	int sockfd, puerto;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[TAM];
	if ( argc < 3 ) {
		fprintf( stderr, "Uso %s host puerto\n", argv[0]);
		exit( 0 );
	}

	puerto = atoi( argv[2] );
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 ) {
		perror( "ERROR apertura de socket" );
		exit( 1 );
	}
	
	server = gethostbyname( argv[1] );
	if (server == NULL) {
		fprintf( stderr,"Error, no existe el host\n" );
		exit( 0 );
	}
	memset( (char *) &serv_addr, '0', sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	bcopy( (char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length );
	serv_addr.sin_port = htons( puerto );

	time_t tiempo = time(0);	//Obtengo la hora de comienzo de ejecucion del proceso
	tlocal= localtime(&tiempo);
	minuto=tlocal->tm_min;
	segundo=tlocal->tm_sec;
	info_satelite(); //Inicializo mi satelite

	if ( connect( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr ) ) < 0 ) {
		perror( "conexion" );
		exit( 1 );
	}

	while(1) 
	{
		memset(buffer, '\0', TAM); 
		
		error_lectura(read( sockfd, buffer, TAM));//Espera instrucciones del server
		if(strcmp(buffer,"update")==0)
			{	
				update(sockfd,argv);
			}
		else if(strcmp(buffer,"scanning")==0)
			{
				scanning(sockfd);
			}
		else if(strcmp(buffer,"telemetria")==0)
			{
				telemetria(argv);
			}
		else
			{
				printf("Fin de la conexion\n");
				exit(0);
			}	
	}
	return 0;
} 


/**
 * @brief Funcion encargada de la recoleccion de datos para llenar la informacion de la estructura satelite.
 * 		   Necesaria para la funcion telemetria.
  */
void info_satelite()
{
	
	char parameter[TAM]={0},pid[10]={0},buffer[TAM]={0};
	FILE *fp;
	memset( satelite.id, '\0', sizeof(satelite.id));
	memset( satelite.uptime, '\0', sizeof(satelite.uptime));
	memset( satelite.memoria, '\0', sizeof(satelite.memoria));
	memset( satelite.cpu, '\0', sizeof(satelite.cpu));
	memset( satelite.version, '\0', sizeof(satelite.version));
	strcpy(satelite.id,"65096A"); //Id del satelite Asterix
	strcpy(satelite.version,"Version312.0"); //Id del satelite Asterix
	// fp = fopen("./satelite_dir/firmware_cliente.bin", "rb");
	// memset(buffer,'\0',sizeof(buffer));
	// fread(buffer, 1, sizeof(buffer) - 1, fp);
	// strcpy(satelite.version,strtok(buffer,"\n"));
	// fclose(fp);
	system("rm ./satelite_dir/info_cliente");
	strcpy(parameter,"ps -Ao vsize,pcpu,pid | grep ");
	sprintf(pid,"%i",getpid()); //Obtengo el pid para filtrar el ps
	strcat(parameter,pid);
	strcat(parameter," >> ./satelite_dir/info_cliente");
	system(parameter);
	
	fp=fopen("./satelite_dir/info_cliente","r");
	memset(buffer,'\0',sizeof(buffer));
	fread(buffer, 1, sizeof(buffer) - 1, fp);
	char *token=strtok(buffer," ");
	fclose(fp);
	if(token != NULL){
		int indice=0;
		while(token != NULL){
			if(indice==0)
				strcpy(satelite.memoria,token);
			if(indice==1)
				strcpy(satelite.cpu,token);
			indice++;	
			token=strtok(NULL," ");
		}
	}

	dif_hora();
	
}


/**
 * @brief Funcion encargada de obtener el tiempo que lleva ejecutandose el proceso satelite en el sistema.
  */
void dif_hora()
{	
	char hora[24]={0},min[5]={0},seg[5]={0};
	time_t tiempo_actual = time(0);
	struct tm *tactual= localtime(&tiempo_actual);
	int minutos= abs(tactual->tm_min -minuto);
	int segundos=tactual->tm_sec;
	if(segundos<segundo)
	{
		minutos=minutos-1;
		segundos=segundos+60;
	}

	sprintf(min,"%i",minutos);
	sprintf(seg,"%i",segundos-segundo);
	strcpy(hora,min);
	strcat(hora,":");
	strcat(hora,seg);
	
	strcpy(satelite.uptime,hora);
}


/**
 * @brief Funcion encargada de la actualizacion del firmware, recibe la actualizacion y se reinicia.
 * @param socketfd y argv. Socket tcp por el cual se inicia y se lleva a cabo la comunicacion. 
 * 						   El segundo parametro es utilizado para el reinicio del proceso.
 */
void update(int sockfd, char *argv[])
{
	char /*buffer[TAM],*/path[TAM];
	//FILE *fp;
	strcpy(path,"./satelite.bin");
	write_ack(sockfd);
	recibir_archivo(sockfd,path,TAM,0);
	// fp = fopen(path, "rb");
	// fread(buffer, 1, sizeof(buffer) - 1, fp);
	// strcpy(satelite.version,strtok(buffer,"\n")); //Actualizo la version de firmware
	// fclose(fp);
	rename("satelite","sateliteold");
	rename("satelite.bin","satelite");
	system("chmod +x satelite");
	//system("mv satelite sateliteold");
	//system("mv satelite.bin satelite");
	printf("Firmware actualizado, reiniciando...\n");
	write_ack(sockfd);
	close(sockfd);
	execvp(argv[0],argv); //Reinicio
}


/**
 * @brief Funcion encargada del envio de la informacion del satelite.
 */
void telemetria(char *argv[])
{	
	int socket_cli;
	int n;
	char buffer[TAM]={"Id_Satelite "};
	struct sockaddr_in struct_cliente;
	socklen_t direccion;
	struct hostent *server;
	direccion=sizeof(struct_cliente);
	info_satelite();
	server = gethostbyname(argv[1]);
	if ( server == NULL ) {
		fprintf( stderr, "ERROR, no existe el host\n");
		exit(0);
	}

	/* Creacion de socket */
	socket_cli = socket( AF_INET, SOCK_DGRAM, 0 );
	if (socket_cli < 0) {
		perror( "apertura de socket" );
		exit( 1 );
	}
	/* Inicialización y establecimiento de la estructura del cliente */
	struct_cliente.sin_family = AF_INET;
	struct_cliente.sin_port = htons( atoi( "8183" ) );
	struct_cliente.sin_addr = *( (struct in_addr *)server->h_addr );
	memset( &(struct_cliente.sin_zero), '\0', 8 );

	strcat(buffer,satelite.id);
	strcat(buffer,"\n");
	strcat(buffer,"Uptime Satelite ");
	strcat(buffer,satelite.uptime);
	strcat(buffer,"\n");
	strcat(buffer,"Version Satelite ");
	strcat(buffer,satelite.version);
	strcat(buffer,"\n");
	strcat(buffer,"Consumo CPU Satelite ");
	strcat(buffer,satelite.cpu);
	strcat(buffer,"\n");
	strcat(buffer,"Consumo memoria Satelite ");
	strcat(buffer,satelite.memoria);
	n=sendto( socket_cli, (void *) buffer, TAM, 0, (struct sockaddr *)&struct_cliente, direccion );
	if(n<0)
	{
		perror("sendto");
	}
	close(socket_cli);
}


/**
 * @brief Funcion encargada del envio de la imagen enviada a la estacion.
 * @param socketfd. Socket tcp por el cual se inicio la comunicacion y por el cual se reciben los datos.
 */
void scanning(int sockfd)
{
	char path[TAM];
	strcpy(path,"./satelite_dir/tierra.jpg");
	write_ack(sockfd);
	enviar_archivo(sockfd,path,1448);
}