/* Cliente en el dominio Unix - orientado a corrientes */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define	COLOR_WHITE "\033[0;0m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"
#define COLOR_RESET   "\x1b[0m"

// #define COLOR_GREEN "\033[0;32m"
// #define	COLOR_WHITE "\033[0;0m"

#define TAM 1024



int autenticacion(int sockfd, char *user_autenticado, char *hostname);
void error_lectura(int);
void error_escritura(int);
void update(int sockfd);
void scanning(int sockfd);
void telemetria(int sockfd);

int main( int argc, char *argv[] ) {
	int sockfd, servlen,n;
	struct sockaddr_un serv_addr;
	char user_autenticado[20];
	char hostname[20];
	char buffer[TAM];

	if (argc < 2) 
	{
		fprintf( stderr, "Uso %s archivo\n", argv[0]);
		fflush(stdout);
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
	
			
	if(autenticacion(sockfd,user_autenticado,hostname)==0)
	{
		printf("No hubo conexion\n");
		fflush(stdout);
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
				printf("Fin de la conexion");
				fflush(stdout);
				exit(0);
			}
			else
			{
				printf("%s",buffer);
				fflush(stdout);
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
		printf("Ingrese su usuario: ");	
		fflush(stdout);
		fgets( buffer, TAM-1, stdin );
		token=strtok(buffer,"\n");
		strcpy(user_aux,token);

		n = write(sockfd, buffer, strlen(buffer));
		error_escritura(n);

		memset(buffer, '\0', TAM);
		n = read(sockfd, buffer, TAM);
		error_lectura(n);

		printf("Respuesta:  %s", buffer);
		fflush(stdout);
		memset( buffer, '\0', TAM );
		fgets( buffer, TAM-1, stdin );

		n=write( sockfd, buffer, strlen(buffer) );
		error_escritura(n);		

		memset( buffer, '\0', TAM );
		n = read( sockfd, buffer, TAM );
		error_lectura(n);	
	
		if(strcmp(buffer,"pass")==0)
		{		
			memset(buffer, '\0', TAM);
			strcpy(user_autenticado,user_aux);

			n = read(sockfd, buffer, TAM);
			error_lectura(n);
			strcpy(hostname,buffer);
			flag=0;
		}
		else if(strcmp(buffer,"fin")==0)
		{
			return 0;
		}
	}
	return 1;
}

void update(int sockfd)
{
	char buffer;
	int flag=-1;
	printf("upd");
	fflush(stdout);
	FILE *fp = fopen("archivoRecibido","w");
	while((flag==recv(sockfd,buffer,1,0))>0)
	{	
		printf("%c",buffer);
		fputc(buffer,fp);
		fflush(stdout);
	}
	fclose(fp);
	printf("sali");
	fflush(stdout);
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