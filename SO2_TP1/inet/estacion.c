#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "funciones.h"

#define TAM 1024
#define NUMERO_USUARIOS 5

#define COLOR_RED "\x1b[31m"
#define COLOR_WHITE "\033[0;0m"
#define COLOR_RESET "\x1b[0m"

struct usuario
{
	char user[10];
	char pass[10];
} usuarios[NUMERO_USUARIOS];

int autenticacion(char *user_autenticado);
void update(int newsockfd);
void scanning(int newsockfd);
void telemetria(int socked_server_udp,struct sockaddr_in struct_servidor);

int main( int argc, char *argv[] ) 
{

	strcpy(usuarios[0].user, "chiqui");
	strcpy(usuarios[0].pass, "030345");
	strcpy(usuarios[1].user, "andres");
	strcpy(usuarios[1].pass, "01234");
	strcpy(usuarios[2].user, "nacho");
	strcpy(usuarios[2].pass, "56789");
	strcpy(usuarios[3].user, "mary");
	strcpy(usuarios[3].pass, "02468");
	strcpy(usuarios[4].user, "floki");
	strcpy(usuarios[4].pass, "13579");

	int sockfd, newsockfd, puerto,  pid, n,autenticado=0;
	unsigned int clilen;
	char buffer[TAM], host[TAM], user_autenticado[20];
	struct sockaddr_in serv_addr, cli_addr;
	
	memset(host, '\0', sizeof(host));
	gethostname(host, sizeof(host));
	if ( argc < 2 )
	{
		fprintf( stderr, "Uso: %s <puerto>\n", argv[0] );
		exit( 1 );
	}

	sockfd = socket( AF_INET, SOCK_STREAM, 0);
	if ( sockfd < 0 ) { 
		perror( " apertura de socket ");
		exit( 1 );
	}

	memset( (char *) &serv_addr, 0, sizeof(serv_addr) );
	puerto = atoi( "8182" );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons( puerto );

	if ( bind(sockfd, ( struct sockaddr *) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		perror( "ligadura" );
		exit( 1 );
	}

	printf( "Proceso: %d - socket disponible: %d\n", getpid(), ntohs(serv_addr.sin_port) );

	listen( sockfd, 2 );
	clilen = sizeof( cli_addr );

	while( 1 ) {
		newsockfd = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if ( newsockfd < 0 ) {
			perror( "accept" );
			exit( 1 );
		}

		pid = fork(); 
		if ( pid < 0 ) {
			perror( "fork" );
			exit( 1 );
		}

		if ( pid == 0 ) // Proceso hijo
		{  
			close( sockfd );

			while ( 1 ) 
			{
				while (autenticado == 0)
				{
					if (autenticacion(user_autenticado))
					{
						autenticado = 1;
					}
					else
					{
						n = write(newsockfd, "fin", 3);
						error_escritura(n);
						printf("Fin de la conexion\n");
						exit(0); //VER COMO MATO AL PADRE
					}
				}
				memset(buffer,'\0',TAM);
				printf("Ingrese la funcion que desea ejecutar\n");
				printf("Funciones disponibles: update - scanning - telemetria \n");
				printf("exit en caso de salir\n");
				printf(COLOR_RED "%s@%s:~ %s" COLOR_RESET, user_autenticado, host, COLOR_WHITE "Ingrese el mensaje a transmitir: " COLOR_RESET);
				fgets(buffer, TAM - 1, stdin);
				strtok(buffer, "\n");
				
				if (strcmp(buffer, "update") == 0)
				{
					update(newsockfd);
				}
				else if (strcmp(buffer, "scanning") == 0)
				{
					scanning(newsockfd);
				}
				else if (strcmp(buffer, "telemetria") == 0)
				{	
					int socked_server_udp,puerto_udp;
					struct sockaddr_in struct_servidor;

					//Creacion del socket
					if((socked_server_udp=socket(AF_INET,SOCK_DGRAM,0))<0)
					{
						perror("socket");
						exit(1);
					}
					
					//Remueve el nombre del archivo si existe
					memset( &struct_servidor, 0, sizeof(struct_servidor) );
					puerto_udp = atoi( "8183" );
					struct_servidor.sin_family = AF_INET;
					struct_servidor.sin_addr.s_addr = INADDR_ANY;
					struct_servidor.sin_port = htons( puerto_udp );
					memset( &(struct_servidor.sin_zero), '\0', 8 );
					
					if( bind( sockfd, (struct sockaddr *) &struct_servidor, sizeof(struct_servidor) ) < 0 )
					{
						perror( "ERROR en binding" );
						exit( 1 );
					}		

					printf( "Socket disponible: %d\n", ntohs(struct_servidor.sin_port) );

					n = write( newsockfd, "telemetria", TAM);
					error_escritura(n);

					telemetria(socked_server_udp,struct_servidor);
				}
				else if (strcmp(buffer, "exit") == 0)
				{
					printf("Fin de la conexion\n");
					autenticado=0; //Vuelvo a la espera de un nuevo usuario
				}
				else
				{
					printf("Comando inexistente\n");
				}
			}
		}
		else {
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
		}
	}
	return 0; 
} 

int autenticacion(char *user_autenticado)
{
	int validez = 0, validez_pass = 0, max = 3, validez_user = 0, aux_i;
	char buffer[TAM];
	char aux_user[TAM];

	while (validez == 0 && max > 0)
	{
		memset(buffer, '\0', TAM);
		memset(aux_user, '\0', TAM);

		printf("Ingrese su usuario \n");
		fgets(buffer, TAM - 1, stdin);
		strtok(buffer, "\n");

		strcpy(aux_user, buffer);

		memset(buffer, '\0', TAM);

		printf("Ingrese su pass \n");
		fgets(buffer, TAM - 1, stdin);
		strtok(buffer, "\n");

		for (int i = 0; i < NUMERO_USUARIOS; i++)
		{
			if (strcmp(aux_user, usuarios[i].user) == 0)
			{
				validez_user = 1;
				aux_i = i;
			}
		}

		if (validez_user == 1)
		{
			if (strcmp(buffer, usuarios[aux_i].pass) == 0)
			{
				validez_pass = 1;
				strcpy(user_autenticado, aux_user);
			}
			else
			{
				validez_pass = 0;
			}
		}

		if (validez_pass == 0 || validez_user == 0) //Alguno de los parametros fue erroneo
		{
			max = max - 1;
			validez_pass = 0; //Inicializo nuevamente
			validez_user = 0;

			if (max == 0)
			{
				printf("Se excedio de intentos, intentelo mas tarde\n");
				return 0;
			}
		}

		else //Ambos parametros son correctos
		{
			validez = 1;
		}
	}

	return 1;
}

void update(int newsockfd)
{
	int n;
	char path[TAM];

	memset(path,'\0',TAM);
	strcpy(path,"/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP1/inet/estacion_dir/firmware.bin");

	n = write( newsockfd, "update", TAM);
	error_escritura(n);
	
	read_ack(newsockfd); //Si tengo dos write seguidos se rompe

	enviar_archivo(newsockfd,path,TAM);

}

void scanning(int newsockfd)
{
	int n;
	char path[TAM];
	memset(path,'\0',TAM);
	strcpy(path,"/home/andres/Facultad/SOII/Andres/Practico/SistemasOperativos2019/SO2_TP1/inet/estacion_dir/escaneo.jpg");
	
	n = write( newsockfd, "scanning", TAM);
	error_escritura(n);
	read_ack(newsockfd); //Si tengo dos write seguidos se rompe
	recibir_archivo(newsockfd,path,64000);
}

void telemetria(int socked_server_udp,struct sockaddr_in struct_servidor)
{	
	socklen_t tamano_direccion;
	char buffer[TAM];
	int n;
	
	tamano_direccion = sizeof( struct_servidor );
	/* Mantenimiento de un lazo infinito, aceptando conexiones */
	memset(buffer,'\0',TAM);
	printf("ESTOY\n");
	n=recvfrom ( socked_server_udp, (void *) buffer, TAM-1, 0, (struct sockaddr *) &struct_servidor, &tamano_direccion);
	if(n<0)
	{
		perror("recvfrom");
	}
	else
	{
		printf("%s\n", buffer);	
	}
	close(socked_server_udp);
}