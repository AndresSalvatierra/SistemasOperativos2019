#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include "funciones.h"

#define NUMERO_USUARIOS 5

#define COLOR_RED "\x1b[31m"
#define COLOR_WHITE "\033[0;0m"
#define COLOR_RESET "\x1b[0m"

struct usuario	//Usuarios validos en el sistema
{
	char user[10];
	char pass[10];
} usuarios[NUMERO_USUARIOS];

int autenticacion(char *user_autenticado);
void update(int newsockfd);
void scanning(int newsockfd);
void telemetria(int socked_server_udp, struct sockaddr_un struct_servidor);

int main(int argc, char *argv[])
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

	int sockfd, newsockfd, servlen, n, autenticado = 0,flag_update; //pid,
	unsigned int clilen;
	struct sockaddr_un cli_addr, serv_addr;
	char buffer[TAM], host[TAM];
	char user_autenticado[20];

	memset(host, '\0', sizeof(host));
	gethostname(host, sizeof(host));	//Usado para el prompt

	if (argc != 2)

	{
		printf("Uso: %s <nombre_de_socket>\n", argv[0]);
		exit(1);
	}

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
	{
		perror("creación de  socket");
		exit(1);
	}

	/* Remover el nombre de archivo si existe */
	unlink(argv[1]);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0)
	{
		perror("ligadura");
		exit(1);
	}

	printf("Proceso: %d - socket disponible: %s\n", getpid(), serv_addr.sun_path);

	listen(sockfd, 2);
	clilen = sizeof(cli_addr);

	while (1)
	{	
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
		if (newsockfd < 0)
		{
			perror("accept");
			exit(1);
		}

		flag_update=0;

		// pid = fork();
		// if (pid < 0)
		// {
		// 	perror("fork");
		// 	exit(1);
		// }

		// if (pid == 0) // proceso hijo
		// {
		// 	close(sockfd);
		while (flag_update==0)
		{	
			while (autenticado == 0)	//Si no estoy autenticado
			{
				if (autenticacion(user_autenticado)) //Autenticacion valida
				{
					autenticado = 1;
				}
				else								//Se excedieron los intentos, cierro el programa
				{
					n = write(newsockfd, "fin", 3);
					error_escritura(n);
					printf("Fin de la conexion\n");
					exit(0); 
				}
			}

			memset(buffer, '\0', TAM);
			printf("Ingrese la funcion que desea ejecutar\n");
			printf("Funciones disponibles: update - scanning - telemetria \n");
			printf("exit en caso de salir\n");
			printf(COLOR_RED "%s@%s:~ %s" COLOR_RESET, user_autenticado, host, COLOR_WHITE "Ingrese el mensaje a transmitir: " COLOR_RESET);
			fgets(buffer, TAM - 1, stdin);
			strtok(buffer, "\n");

			if (strcmp(buffer, "update") == 0)	//Determinacion de que hacer
			{
				update(newsockfd);
				flag_update=1;
			}
			else if (strcmp(buffer, "scanning") == 0)
			{
				scanning(newsockfd);
			}
			else if (strcmp(buffer, "telemetria") == 0)
			{
				int socked_server_udp;
				struct sockaddr_un struct_servidor;

				char argv[TAM] = {"./teludp"};

				//Creacion del socket udp 
				if ((socked_server_udp = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
				{
					perror("socket");
					exit(1);
				}

				//Remueve el nombre del archivo si existe
				unlink(argv);

				/* Inicialización y establecimiento de la estructura del servidor */
				memset(&struct_servidor, 0, sizeof(struct_servidor));
				struct_servidor.sun_family = AF_UNIX;
				strncpy(struct_servidor.sun_path, argv, sizeof(struct_servidor.sun_path));

				n = write(newsockfd, "telemetria", TAM);
				error_escritura(n);

				/* Ligadura del socket de servidor a una dirección */
				if ((bind(socked_server_udp, (struct sockaddr *)&struct_servidor, SUN_LEN(&struct_servidor))) < 0)
				{
					perror("bind");
					exit(1);
				}


				telemetria(socked_server_udp, struct_servidor);
			}
			else if (strcmp(buffer, "exit") == 0)
			{
				autenticado = 0; 				//Vuelvo a la espera de un nuevo usuario
				printf("Fin de la conexion\n");
				sleep(5);
				//exit(0);
			}
			else
			{
				printf("Comando inexistente\n");
			}
		}
	}

		// else
		// {
		// 	printf("SERVIDOR: Nuevo cliente, que atiende el proceso hijo: %d\n", pid);
		// 	close(newsockfd);
		// }
	//}
	return 0;
}

/**
 * @brief Funcion encargada de la autenticacion del cliente.
 * @param user_atenticado. De ser valida la autenticacion devuelve el usuario para utilizarlo en el prompt
 * @return 1 en caso de validarse, 0 en caso de no ser valido.
 */
int autenticacion(char *user_autenticado)
{
	int validez = 0, validez_pass = 0, max = 3, validez_user = 0, aux_i;
	char buffer[TAM];
	char aux_user[TAM];

	while (validez == 0 && max > 0)	//Mientras no sea valido y no supere el maximo de intentos
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

			if (max == 0) //Alcanzo el maximo de intentos
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

/**
 * @brief Funcion encargada de la actualizacion del firmware del satelite, lee un archivo binario y lo envia.
 * @param socketfd. Socket tcp por el cual se inicia y se lleva a cabo la comunicacion.
 */
void update(int newsockfd)
{
	int n;
	char path[TAM];

	memset(path, '\0', TAM);
	strcpy(path, "./estacion_dir/firmware.bin");

	n = write(newsockfd, "update", TAM);
	error_escritura(n);

	read_ack(newsockfd); 

	enviar_archivo(newsockfd, path, TAM); 
	error_escritura(n);
	close(newsockfd);
}

/**
 * @brief Funcion encargada de la recepcion de la informacion del satelite.
 * @param socket udp y estructura del socket
 */
void telemetria(int socked_server_udp, struct sockaddr_un struct_servidor)
{
	socklen_t tamano_direccion;
	char buffer[TAM];
	int n;
	printf("Socket disponible: %s\n", struct_servidor.sun_path);

	tamano_direccion = sizeof(struct_servidor);
	
	memset(buffer, '\0', TAM);
	n = recvfrom(socked_server_udp, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&struct_servidor, &tamano_direccion);
	if (n < 0)
	{
		perror("recvfrom");
	}
	else
	{
		printf("%s\n", buffer);
	}
	close(socked_server_udp);
}

/**
 * @brief Funcion encargada de la recepcion de la imagen enviada por el satelite.
 * @param socketfd. Socket tcp por el cual se inicio la comunicacion y por el cual se reciben los datos.
 */
void scanning(int newsockfd)
{
	int n;
	char path[TAM];
	memset(path, '\0', TAM);
	strcpy(path, "./estacion_dir/escaneo.jpg");

	n = write(newsockfd, "scanning", TAM);
	error_escritura(n);
	read_ack(newsockfd); 
	recibir_archivo(newsockfd, path, 1400);
}