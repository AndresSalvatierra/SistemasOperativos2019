void write_ack(int sock); 
void read_ack(int sock);
void error_escritura(int n);
void error_lectura(int n);
void enviar_archivo(int sockfd,char *path,int tam);
void recibir_archivo(int sockfd,char *path, int tam, int tipo);

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
	int tam_file_send, n,tam_file,paquete=1;
	char buffer[tam];

	fp=fopen(path,"rb");
	if (fp == NULL)
	{
		printf("Error al abrir el file\n");
		exit(1);
	}
	
	fseek(fp, 0, SEEK_END); //Posiciona el puntero en SEEK_END(final del file)
	tam_file_send = ftell(fp); //Obtengo el tamaño del file
	fseek(fp, 0, SEEK_SET); //Posiciona el puntero en SEEK_SET(inicio del file)
	n=write(sockfd, &tam_file_send, sizeof(tam_file_send)); //Envio el tamanio de file
	error_escritura(n);

	read_ack(sockfd);

	while(!feof(fp))	//Si no llegue al final del archivo
	{	
		memset(buffer,'\0',sizeof(buffer));//Quiero mandar mas de un paquete lo reinicializo
		tam_file = fread(buffer, 1, sizeof(buffer), fp); //Obtengo el tamaño a mandar y lo que voy a mandar guardo en buffer

		n=write(sockfd, buffer, tam_file);
		error_escritura(n);
		paquete=paquete+1;
	}
	fclose(fp);	
}

/**
 * @brief Funcion encargada de la recepcion de archivos. Primero se recibe el tamaño de datos que voy a recibir, hasta no alcanzar este maximo no dejo de recibir.
 * 		   Es utilizada en la funcion update y en la funcion scanning.
 * @param sockfd, path, tam, tipo. Es el socket por el cual se lleva a cabo la comunicacion, el path en el cual voy a guardar el archivo,
 * 							 el tamaño maximo del buffer donde voy a cargar la informacion a enviar y el tipo de funcion que lo llama simplemente para impresiones
 * 							(1) scanning (0) update
 */
void recibir_archivo(int sockfd,char *path, int tam, int tipo)
{
	int tam_total = 0,n, tam_file_recv = 0, tam_file=0,paquete = 0;
	char buffer[tam];

	
	n=read(sockfd, &tam_file_recv, sizeof(tam_file_recv)); //Obtengo el tamaño del file
	error_lectura(n);

	write_ack(sockfd);

	if(tipo==1)
	{
		printf("El tamanio del paquete a recibir %i\n",tam_file_recv);
	}
	

	FILE *fp = fopen(path, "wb");

	if (fp == NULL)
	{
		printf("Error para abrir el archivo\n");
		exit(1);
	}

	while(tam_total<tam_file_recv)
	{	
		memset(buffer,'\0',sizeof(buffer));
		tam_file = read(sockfd, buffer, sizeof(buffer));
		fwrite(buffer, 1, tam_file, fp);
		tam_total += tam_file;
		paquete=paquete+1;
	}
	fclose(fp);
	
	if(tipo==1)
	{
		printf("Cantidad de paquetes recibidos: %i\n", paquete);
	}
	printf("Archivo recibido\n");
	
}
