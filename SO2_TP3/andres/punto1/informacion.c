#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/sysinfo.h>

#define TAM 1024

/*struct embebido		//Informacion referida al embebido
{
	char cpu[20];
	char memoria[20];
};*/


//static struct embebido raspi;
static struct tm *tlocal;

void info_embebido();

int main(int argc, char *argv[])
{	
    time_t tiempo = time(0);	//Obtengo la hora de comienzo de ejecucion del proceso
	tlocal= localtime(&tiempo);
    
	printf("\n Fecha y hora actual: \n %s \n", ctime(&tiempo));

	info_embebido();

    //printf("Memoria en uso: %s\n",raspi.memoria);
    //printf("Consumo de CPU: %s\n",raspi.cpu);

}

void info_embebido()
{
    char parameter[TAM]={0},pid[10]={0},buffer[TAM]={0};
    struct sysinfo s_info;
    int error= sysinfo(&s_info);
    if(error!=0){
    printf("error=%d\n",error);
    }
    FILE *fp;
	//memset( raspi.memoria, '\0', sizeof(raspi.memoria));
	//memset( raspi.cpu, '\0', sizeof(raspi.cpu));
	
	printf("\n Uptime: %ld segundos \n",s_info.uptime);
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
				//strcpy(raspi.memoria,token);
				printf("Memoria en uso: %s \n",token);
			if(indice==1)
				//strcpy(raspi.cpu,token);
				printf("Cpu en uso: %s \n",token);
			indice++;	
			token=strtok(NULL," ");
		}
	}

	printf("\n");
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
}

