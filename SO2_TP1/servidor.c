#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h> 

#define TAM 1024
#define NUMERO_CLIENTES 5

struct usuario
{
	char user[10];
	char pass[10];
}usuarios[NUMERO_CLIENTES];

void error_lectura(int);
void error_escritura(int);
int autenticacion (int newsockfd);
void write_ack(int sockfd); 
void read_ack(int sockfd);
void update(int newsockfd);
void scanning(int newsockfd);
void telemetria(int newsockfd);

int main(int argc, char *argv[]) {

	//system("top ");
	//system("top -p $(grep -d, cliente) >> hola");
	
	strcpy(usuarios[0].user,"chiqui");
	strcpy(usuarios[0].pass,"030345");
	strcpy(usuarios[1].user,"andres");
	strcpy(usuarios[1].pass,"01234");
	strcpy(usuarios[2].user,"nacho");
	strcpy(usuarios[2].pass,"56789");
	strcpy(usuarios[3].user,"mary");
	strcpy(usuarios[3].pass,"02468");
	strcpy(usuarios[4].user,"floki");
	strcpy(usuarios[4].pass,"13579");
	
	int sockfd, newsockfd, servlen, pid,n;
	unsigned int clilen;
	struct sockaddr_un  cli_addr, serv_addr;
	char buffer[TAM];

	/* Se toma el nombre del socket de la línea de comandos */
	if(argc != 2)
	{
		printf( "Uso: %s <nombre_de_socket>\n", argv[0] );
		exit( 1 );
	}

	if (( sockfd = socket( AF_UNIX, SOCK_STREAM, 0)) < 0) 
	{
		perror( "creación de  socket");
		exit(1);
	}

	/* Remover el nombre de archivo si existe */
	unlink (argv[1]);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if( bind(sockfd,(struct sockaddr *)&serv_addr,servlen )<0)
	{
		perror( "ligadura" ); 
		exit(1);
	}

	printf( "Proceso: %d - socket disponible: %s\n", getpid(), serv_addr.sun_path );

	listen(sockfd, NUMERO_CLIENTES);
	clilen = sizeof( cli_addr );

	while ( 1 )
	{
		newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
		{
			perror("accept");
			exit(1);
		}

		pid = fork();
                
		if (pid < 0) 
		{
			perror("fork");
			exit(1);
		}
		
		if ( pid == 0 ) // proceso hijo
		{        
			close(sockfd);
			if(autenticacion(newsockfd)==1)	//Verificacion de usuario
			{
				int flag=1;
				while(flag)
				{
					memset(buffer, '\0', TAM );
					n = read( newsockfd, buffer, TAM); //Espera instrucciones del cliente
					error_lectura(n);

					strtok(buffer,"\n");		
					
					if(strcmp(buffer,"update")==0)
					{	
						update(newsockfd);
					}
					else if(strcmp(buffer,"scanning")==0)
					{
						scanning(newsockfd);
					}
					else if(strcmp(buffer,"telemetria")==0)
					{
						telemetria(newsockfd);
					}
					else if(strcmp(buffer,"fin")==0)
					{
						n = write( newsockfd, "fin", 3);
						error_escritura(n);
						printf("Fin de la conexion con cliente\n");
						exit(0);
					}
					else
					{
						n = write( newsockfd, "No existe esa funcion", TAM);
						error_escritura(n);
					}
				}	
				
			}

			else
			{	
				n = write( newsockfd, "Usuario denegado", 18);
				error_escritura(n);
				printf("Usuario denegado \n");
				exit(0);
			}
		}
		
		else 
		{
			printf( "SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid );
			close( newsockfd );
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

int autenticacion (int newsockfd)
{	
	int n,validez=0,validez_pass=0,max=3, validez_user=0,aux_i;
	char buffer[TAM];
	char aux_user[TAM];
	char host[1024];

	while(validez==0 && max>0)
	{	
		memset(buffer, '\0', TAM );
		memset(aux_user, '\0', TAM);

		n = read( newsockfd, buffer, TAM);
		error_lectura(n);

		strtok(buffer,"\n");
		strcpy(aux_user,buffer);

		n = write( newsockfd, "Ingrese su pass", 18 );
		error_escritura(n);

		memset( buffer, '\0', TAM );
		n = read( newsockfd, buffer, TAM );
		error_lectura(n);
		strtok(buffer,"\n");
	
		for(int i=0;i<NUMERO_CLIENTES;i++)
		{
			
			if(strcmp(aux_user,usuarios[i].user)==0)
			{
				validez_user=1;
				aux_i=i;
			}
		}

		if(validez_user==1)
		{	
			if(strcmp(buffer,usuarios[aux_i].pass)==0)
			{
				validez_pass=1;
			}
			else
			{
				validez_pass=0;
			}
		}

		if(validez_pass==0 || validez_user==0) //Alguno de los parametros fue erroneo
		{
			max=max-1;
			validez_pass=0;	//Inicializo nuevamente
			validez_user=0;
			
			if(max!=0)
			{
				n = write(newsockfd, "no", 3);
				error_escritura(n);
			}
		}
		
		else //Ambos parametros son correctos
		{	
			validez=1;
			n = write(newsockfd, "pass", 5);
			error_escritura(n);

			read_ack(newsockfd);

			memset( host, '\0', sizeof(host));
			gethostname(host,sizeof(host));

			n = write(newsockfd, host, sizeof(host));
			error_escritura(n);
			
		}
	}

	if(validez==1) //Autenticacion correcta
	{
		return 1;
	}

	n = write(newsockfd, "fin", 5);
	error_escritura(n);

	read_ack(newsockfd);
	return 0;
}

void update(int newsockfd)
{
	FILE *fp;
	int size_file, n,read_size,packet_index=1;
	char buffer[TAM];
	

	memset(buffer,'\0',sizeof(buffer));
	n = write( newsockfd, "update", TAM);
	error_escritura(n);
	

	read_ack(newsockfd);

	fp = fopen("./firmware.bin", "rb");

	if (fp == NULL)
	{
		printf("Error al abrir el file\n");
		exit(1);
	}

	fseek(fp, 0, SEEK_END); //Posiciona el puntero en SEEK_END(final del file)
	size_file = ftell(fp); //Obtengo el tamaño del file
	fseek(fp, 0, SEEK_SET); //Posiciona el puntero en SEEK_SET(inicio del file)
	n=write(newsockfd, &size_file, sizeof(size_file)); //Envio el tamanio de file
	error_escritura(n);

	read_ack(newsockfd);
	while(!feof(fp))
	{
		printf("FEOF %i\n",feof(fp));
		read_size = fread(buffer, 1, sizeof(buffer) - 1, fp); //Obtengo el tamaño a mandar y lo que voy a mandar guardo en buffer

		n = write(newsockfd, buffer, read_size);
		error_escritura(n);
		printf("Read size %i\n", read_size);
		printf("n %i\n", n);
		printf("Numero de paquete: %i \n",packet_index);
		packet_index=packet_index+1;
		//memset(buffer,'\0',sizeof(buffer));//Quiero mandar mas de un paquete lo reinicializo
		bzero(buffer, sizeof(buffer));
	}
	fclose(fp);
}

void scanning(int newsockfd)
{	
	int n;
	n = write( newsockfd, "scanning", TAM);
	error_escritura(n);
	printf("sca");
	fflush(stdout);
}

void telemetria(int newsockfd)
{	
	int n;
	char buffer[TAM];
	n = write( newsockfd, "telemetria", TAM);
	error_escritura(n);
	
	for (int i=0; i<10; i++)
	{	
		memset( buffer, '\0', TAM );
		n = read( newsockfd, buffer, TAM );
		error_lectura(n);
		printf("%s\n", buffer);
		write_ack(newsockfd);
	}
}


void write_ack(int newsockfd)
{
	static char buffer[] ="ack";
	static int n;
	n=write( newsockfd, buffer, strlen(buffer) );
	error_escritura(n);		
}

void read_ack(int newsockfd)
{
	char buffer[TAM];
	int n;
	memset( buffer, '\0', TAM );
	n = read( newsockfd, buffer, TAM );
	error_lectura(n);

	if(strcmp(buffer,"ack")!=0)	
	{
		exit(1);
	}
}