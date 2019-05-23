#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <sys/un.h>
#include <unistd.h>

#define TAM 1024

struct embebido		//Informacion referida al embebido
{
	char uptime[20];	
	char cpu[20];
	char memoria[20];
};


static struct embebido raspi;
static struct tm *tlocal;
static int minuto,segundo;

void info_embebido();
void dif_hora();

int main(int argc, char *argv[])
{	
    time_t tiempo = time(0);	//Obtengo la hora de comienzo de ejecucion del proceso
	tlocal= localtime(&tiempo);
	minuto=tlocal->tm_min;
	segundo=tlocal->tm_sec;	

	printf("Content-Type: text/plain;charset=us-ascii\n\n");
    
	info_embebido();

    printf("Uptime: %s\n",raspi.uptime);
    printf("Memoria en uso: %s\n",raspi.memoria);
    printf("Consumo de CPU: %s\n",raspi.cpu);

}

void info_embebido()
{
    char parameter[TAM]={0},pid[10]={0},buffer[TAM]={0};
	FILE *fp;
	memset( raspi.uptime, '\0', sizeof(raspi.uptime));
	memset( raspi.memoria, '\0', sizeof(raspi.memoria));
	memset( raspi.cpu, '\0', sizeof(raspi.cpu));
    
	//system("rm ./info");
	strcpy(parameter,"ps -Ao vsize,pcpu,pid | grep ");
	sprintf(pid,"%i",getpid()); //Obtengo el pid para filtrar el ps
	strcat(parameter,pid);
	strcat(parameter," >> ./info");
	system(parameter);

    fp=fopen("./info","r");
	memset(buffer,'\0',sizeof(buffer));
	fread(buffer, 1, sizeof(buffer) - 1, fp);
	char *token=strtok(buffer," ");
	fclose(fp);

    if(token != NULL){
		int indice=0;
		while(token != NULL){
			if(indice==0)
				strcpy(raspi.memoria,token);
			if(indice==1)
				strcpy(raspi.cpu,token);
			indice++;	
			token=strtok(NULL," ");
		}
	}

	printf("Informacion del procesador: \n");

	FILE *cpuinfoFile;
	cpuinfoFile = fopen ("/proc/cpuinfo", "r");

	if (cpuinfoFile==NULL) {
		printf("No cpuinfo file found!");
		fclose(cpuinfoFile);
		return;
	}

	while(!feof(cpuinfoFile)) {
		char *modelCpu;
		char *procesador;
		char *lineRead = fgets(buffer, 50, cpuinfoFile);

		if(lineRead==NULL) { break; }

		if((procesador = strstr(lineRead, "processor"))!= NULL)
			printf("%s \n",procesador);

		if((modelCpu = strstr(lineRead, "model name"))!= NULL)
			printf("%s \n",modelCpu);

	}	

	dif_hora();
}

/**
 * @brief Funcion encargada de obtener el tiempo que lleva ejecutandose el proceso satelite en el sistema.
  */
void dif_hora()
{	

	char hora[24]={0},min[5]={0},seg[5]={0},actual[30]={0};
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
	strcpy(raspi.uptime,hora);

	printf("\n Fecha y hora actual: \n %s \n", ctime(&tiempo_actual));
}
