void write_ack(int sock); 
void read_ack(int sock);
void error_escritura(int n);
void error_lectura(int n);
void enviar_archivo(int sockfd,char *path,int tam);
void recibir_archivo(int sockfd,char *path, int tam);

#define TAM 1024
/**
 * @brief Funcion encargada de la verificacion de la lectura del socket. En caso de error cierra el programa.
 * @param entero. Representa los bytes enviados o -1 en caso de error.
 */
void error_lectura(int n){
	if ( n < 0 ) {
		perror( "lectura de socket" );
		exit(1);
	}
}
/**
 * @brief Funcion encargada de la verificacion de la escritura del socket. En caso de error cierra el programa.
 * @param entero. Representa los bytes enviados o -1 en caso de error.
 */
void error_escritura(int n){
	if ( n < 0 ) {
		perror( "escritura de socket" );
		exit(1);
	}

}

/**
 * @brief Funcion encargada del envio de acknowledge, utilizada en caso de tener dos read seguidos.
 * @param socketfd. Socket por el cual se lleva a cabo la comunicacion.
 */
void write_ack(int sockfd)
{
	char buffer []="ack";
	int n;
	n=write( sockfd, buffer, strlen(buffer) );
	error_escritura(n);		
}

/**
 * @brief Funcion encargada de la recepcion de acknowledge, utilizada en caso de tener dos write seguidos.
 * @param socketfd. Socket por el cual se lleva a cabo la comunicacion.
 */
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

/**
 * @brief Funcion encargada del envio de archivos, se envia primero el tamaño del tamaño a enviar y se sigue enviando hasta llegar al final del archivo.
 * 		  Es utilizada en la funcion update y en la funcion scanning.
 * @param sockfd, path, tam. Es el socket por el cual se lleva a cabo la comunicacion, el path en el cual tengo ubicado el archivo
 * 							 y por ultimo el tamaño maximo del buffer donde voy a cargar la informacion a enviar.
 */
void enviar_archivo(int sockfd,char *path,int tam)
{
	FILE *fp;
	int size_file, n,read_size,packet_index=1;
	char buffer[tam];

	fp=fopen(path,"rb");
	if (fp == NULL)
	{
		printf("Error al abrir el file\n");
		exit(1);
	}

	fseek(fp, 0, SEEK_END); //Posiciona el puntero en SEEK_END(final del file)
	size_file = ftell(fp); //Obtengo el tamaño del file
	fseek(fp, 0, SEEK_SET); //Posiciona el puntero en SEEK_SET(inicio del file)
	n=write(sockfd, &size_file, sizeof(size_file)); //Envio el tamanio de file
	error_escritura(n);

	read_ack(sockfd);

	while(!feof(fp))
	{	
		memset(buffer,'\0',sizeof(buffer));//Quiero mandar mas de un paquete lo reinicializo
		printf("FEOF %i\n",feof(fp));
		read_size = fread(buffer, 1, sizeof(buffer) - 1, fp); //Obtengo el tamaño a mandar y lo que voy a mandar guardo en buffer

		n = write(sockfd, buffer, read_size);
		error_escritura(n);
		printf("Read size %i\n", read_size);
		printf("n %i\n", n);
		printf("Numero de paquete: %i \n",packet_index);
		packet_index=packet_index+1;
	}
	fclose(fp);
}

/**
 * @brief Funcion encargada de la recepcion de archivos. Primero se recibe el tamaño de datos que voy a recibir, hasta no alcanzar este maximo no dejo de recibir.
 * 		   Es utilizada en la funcion update y en la funcion scanning.
 * @param sockfd, path, tam. Es el socket por el cual se lleva a cabo la comunicacion, el path en el cual voy a guardar el archivo
 * 							 y por ultimo el tamaño maximo del buffer donde voy a cargar la informacion a enviar.
 */
void recibir_archivo(int sockfd,char *path, int tam)
{
	int recv_size = 0,n , size_file_recv = 0, read_size, write_size, packet_index = 1;
	char buffer[tam];
	n= read(sockfd, &size_file_recv, sizeof(size_file_recv)); //Obtengo el tamaño del file
	error_lectura(n);

	write_ack(sockfd);

	printf("%i\n",size_file_recv);

	FILE *fp = fopen(path, "wb");

	if (fp == NULL)
	{
		printf("Error para abrir el archivo\n");
		exit(1);
	}

	while(recv_size<size_file_recv)
	{	
		memset(buffer,'\0',tam);
		read_size = read(sockfd, buffer, tam);

		printf("Packet number received: %i\n", packet_index);
		printf("Packet size: %i\n", read_size);
		//write_ack(sockfd);
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
}
