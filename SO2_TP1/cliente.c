/* Cliente en el dominio Unix - orientado a corrientes */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <string.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define	COLOR_WHITE "\033[0;0m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

#define TAM 1024

struct satelites
{
	char id[20];
	char uptime[20];	
	char version[20];
	char cpu[20];
	char memoria[20];
};

static struct satelites satelite;

int autenticacion(int sockfd, char *user_autenticado, char *hostname);
void error_lectura(int n);
void inicializacion();
void error_escritura(int n);
void write_ack(int sockfd); 
void read_ack(int sockfd);
void update(int sockfd);
void scanning(int sockfd);
void telemetria(int sockfd);

int main( int argc, char *argv[] ) {
	int sockfd, servlen,n;
	struct sockaddr_un serv_addr;
	char user_autenticado[20];
	char hostname[20];
	char buffer[TAM];
	sleep(5);
	system("ps -Ao pcpu,vsize,utime,pid | grep 12059 >> holis");

	if (argc < 2) 
	{
		fprintf( stderr, "Uso %s archivo\n", argv[0]);
		exit( 0 );
	}

	memset((char *)&serv_addr, '\0', sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	servlen = strlen( serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) 
	{
		perror( "creación de socket" );
		exit( 1 );
	}

 	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen ) < 0) 
	{
		perror( "conexión" );
		exit( 1 );
	}
	
	inicializacion(); //Inicializo mi satelite

	if(autenticacion(sockfd,user_autenticado,hostname)==0)
	{
		memset( buffer, '\0', TAM );
		n = read( sockfd, buffer, TAM );
		error_lectura(n);
		printf("Respuesta: %s\n", buffer);
		exit(0);
	}
	else
	{
		while(1){
			memset(buffer, '\0', TAM); 
			printf(COLOR_RED"%s@%s:~ %s"COLOR_RESET,user_autenticado,hostname,COLOR_WHITE"Ingrese el mensaje a transmitir: "COLOR_RESET);
			fflush(stdout);
			fgets( buffer, TAM-1, stdin );
			n = write( sockfd, buffer, TAM);
			error_escritura(n);

			memset(buffer, '\0', TAM); 
			n = read( sockfd, buffer, TAM);//Espera instrucciones del server
			error_lectura(n);

			if(strcmp(buffer,"update")==0)
			{	
				update(sockfd);
			}
			else if(strcmp(buffer,"scanning")==0)
			{
				scanning(sockfd);
			}
			else if(strcmp(buffer,"telemetria")==0)
			{
				telemetria(sockfd);
			}
			else if(strcmp(buffer,"fin")==0)
			{
				printf("Fin de la conexion\n");
				exit(0);
			}
			else
			{
				printf("Respuesta del server: %s \n",buffer);
			}
		}
		
	}
	

	return 0;
			
}

void error_lectura(int n){
	if ( n < 0 ) {
		perror( "lectura de socket" );
		exit(1);
	}
}

void error_escritura(int n){
	if ( n < 0 ) {
		perror( "escritura de socket" );
		exit(1);
	}

}
/**
*@brief lo que hace la funcion
*@author
*@param
*@date
*@return
*/
int autenticacion(int sockfd,char *user_autenticado,char *hostname)
{
	int n,flag=1;
	char buffer[TAM],user_aux[TAM];
	char *token;
	while(flag)
	{
		memset( buffer, '\0', TAM );
		printf("Ingrese su usuario: \n");	
		fgets( buffer, TAM-1, stdin );
		token=strtok(buffer,"\n");
		strcpy(user_aux,token);

		n = write(sockfd, buffer, strlen(buffer));
		error_escritura(n);

		memset(buffer, '\0', TAM);
		n = read(sockfd, buffer, TAM);
		error_lectura(n);

		printf("Respuesta del server:  %s\n", buffer);
		memset( buffer, '\0', TAM );
		fgets( buffer, TAM-1, stdin );

		n=write( sockfd, buffer, strlen(buffer) );
		error_escritura(n);		

		memset( buffer, '\0', TAM );
		n = read( sockfd, buffer, TAM );
		error_lectura(n);	

		if(strcmp(buffer,"pass")==0)
		{	
			write_ack(sockfd);

			memset(buffer, '\0', TAM);
			strcpy(user_autenticado,user_aux);

			n = read(sockfd, buffer, TAM);
			error_lectura(n);
			strcpy(hostname,buffer);
			flag=0;
		}
		else if(strcmp(buffer,"fin")==0)
		{
			write_ack(sockfd);
			return 0;
		}
		
	}
	return 1;
}

void update(int sockfd)
{
	int recv_size = 0,n , size_file_recv = 0, read_size, write_size, packet_index = 1;

	char buffer[TAM];
	FILE *fp;
	
	write_ack(sockfd);

	n= read(sockfd, &size_file_recv, sizeof(size_file_recv)); //Obtengo el tamaño del file
	error_lectura(n);

	fp = fopen("firmware_cliente.bin", "wb");

	if (fp == NULL)
	{
		printf("Error para abrir el archivo\n");
		exit(1);
	}

	while(recv_size<size_file_recv)
	{
		memset(buffer,'\0',sizeof(buffer));
		read_size = read(sockfd, buffer, TAM);
		
		printf("Packet number received: %i\n", packet_index);
		printf("Packet size: %i\n", read_size);

			//Write the currently read data into our image file
		write_size = fwrite(buffer, 1, read_size, fp);
		printf("Written image size: %i\n", write_size);

		if (read_size != write_size)
		{
			printf("Error en la actualizacion\n");
			exit(1);
		}

			//Increment the total number of bytes read
		recv_size += read_size;
		packet_index=packet_index+1;
		printf("Total received image size: %i\n", recv_size);
		printf(" \n");
		printf(" \n");
	}

	fclose(fp);
	printf("Image successfully Received!\n");
	strcpy(satelite.version,buffer); //Actualizo la version de firmware
	printf("Firmware actualizado, reiniciando...\n");
}

void inicializacion()
{
	memset( satelite.id, '\0', sizeof(satelite.id));
	memset( satelite.uptime, '\0', sizeof(satelite.uptime));
	memset( satelite.memoria, '\0', sizeof(satelite.memoria));
	memset( satelite.cpu, '\0', sizeof(satelite.cpu));
	strcpy(satelite.id,"65096A"); //Id del satelite Asterix
	strcpy(satelite.memoria,"hola");
	strcpy(satelite.cpu,"chau");
	strcpy(satelite.uptime,"JEJE");
	FILE *fp = fopen("firmware_cliente.bin", "rb");
	char buffer[TAM];
	memset(buffer,'\0',sizeof(buffer));
	fread(buffer, 1, sizeof(buffer) - 1, fp);
	strcpy(satelite.version,buffer);
	fclose(fp);
}

void scanning(int sockfd)
{
	printf("sca");
	fflush(stdout);
}

void telemetria(int sockfd)
{
	printf("tel");
	fflush(stdout);
}

void write_ack(int sockfd)
{
	char buffer []="ack";
	int n;
	n=write( sockfd, buffer, strlen(buffer) );
	error_escritura(n);		
}

void read_ack(int sockfd)
{
	char buffer[TAM];
	int n;
	memset( buffer, '\0', TAM );
	n = read( sockfd, buffer, TAM );
	error_lectura(n);

	if(strcmp(buffer,"ack")!=0)	
	{
		exit(1);
	}
}
