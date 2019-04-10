#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include "funciones.h"
#define TAM 1024


struct satelites		//Informacion referida al satelite
{
	char id[20];
	char uptime[20];	
	char version[20];
	char cpu[20];
	char memoria[20];
};

static struct satelites satelite;
static struct tm *tlocal;
static int minuto,segundo;

void update(int sockfd, char *argv[]);
void info_satelite();
void telemetria();
void dif_hora();
void scanning(int sockfd);

int main(int argc, char *argv[])
{	
	
	int sockfd, servlen,n;
	struct sockaddr_un serv_addr;
	char buffer[TAM];
	if (argc < 2)
	{
		fprintf(stderr, "Uso %s archivo\n", argv[0]);
		exit(0);
	}

	memset((char *)&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("creación de socket");
		exit(1);
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
	{
		perror("conexión");
		exit(1);
	}
	
	time_t tiempo = time(0);	//Obtengo la hora de comienzo de ejecucion del proceso
	tlocal= localtime(&tiempo);
	minuto=tlocal->tm_min;
	segundo=tlocal->tm_sec;	

	info_satelite(); //Inicializo mi satelite
	
	while (1)
	{
		memset(buffer, '\0', TAM); 
		n = read( sockfd, buffer, TAM);//Espera instrucciones del server
		error_lectura(n);

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
				telemetria();
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
 * @brief Funcion encargada de la actualizacion del firmware, recibe la actualizacion y se reinicia.
 * @param socketfd y argv. Socket tcp por el cual se inicia y se lleva a cabo la comunicacion. 
 * 						   El segundo parametro es utilizado para el reinicio del proceso.
 */
void update(int sockfd, char *argv[])
{
	char buffer[TAM],path[TAM];
	FILE *fp;
	strcpy(path,"./satelite_dir/firmware_cliente.bin");
	write_ack(sockfd);
	recibir_archivo(sockfd,path,TAM);
	fp = fopen(path, "rb");
	fread(buffer, 1, sizeof(buffer) - 1, fp);
	strcpy(satelite.version,strtok(buffer,"\n")); //Actualizo la version de firmware
	fclose(fp);
	printf("Firmware actualizado, reiniciando...\n");
	close(sockfd);
	execvp(argv[0],argv); //Reinicio
}

/**
 * @brief Funcion encargada de la recoleccion de datos para llenar la informacion de la estructura satelite.
 * 		   Necesaria para la funcion telemetria.
  */
void info_satelite()
{
	char parameter[TAM]={0},pid[10]={0},buffer[TAM]={0};
	FILE *fp;
	int indice=0;
	memset( satelite.id, '\0', sizeof(satelite.id));
	memset( satelite.uptime, '\0', sizeof(satelite.uptime));
	memset( satelite.memoria, '\0', sizeof(satelite.memoria));
	memset( satelite.cpu, '\0', sizeof(satelite.cpu));
	strcpy(satelite.id,"65096A"); //Id del satelite Asterix
	
	
	fp = fopen("./satelite_dir/firmware_cliente.bin", "rb");
	memset(buffer,'\0',sizeof(buffer));
	fread(buffer, 1, sizeof(buffer) - 1, fp);
	strcpy(satelite.version,strtok(buffer,"\n"));
	fclose(fp);

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
	
	if(token != NULL){
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
 * @brief Funcion encargada del envio de la informacion del satelite.
 */
void telemetria()
{	

	int socket_cli;
	int n;
	
	char buffer[TAM]={"Id_Satelite "};
	char argv[TAM]={"./teludp"};
	struct sockaddr_un struct_cliente;
	socklen_t direccion;			
	direccion=sizeof(struct_cliente);
	info_satelite();
	/* Creacion de socket */
	if(( socket_cli = socket(AF_UNIX, SOCK_DGRAM, 0))< 0) 
	{
		perror("socket");
	}
	/* Inicialización y establecimiento de la estructura del cliente */
	memset( &struct_cliente, 0, sizeof( struct_cliente ) );
	struct_cliente.sun_family = AF_UNIX;
	strncpy( struct_cliente.sun_path, argv, sizeof( struct_cliente.sun_path ) );
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
	n=sendto( socket_cli, (void *) buffer, sizeof(buffer), 0, (struct sockaddr *)&struct_cliente, direccion );
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