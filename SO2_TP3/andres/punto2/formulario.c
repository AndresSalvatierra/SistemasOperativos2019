#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <dirent.h>
#include <sys/sysinfo.h>
int main(int argc, char* argv[]) {
char* readInput = strchr(getenv("QUERY_STRING"),'=')+1;

char *inputParse = strdup(readInput);
char anio[10]={0};
char dia[10]={0};

strcat(anio,inputParse);
strcat(dia,inputParse);

char *year=strtok(dia,"&");
printf("El anio es %s \n",year);

char *day=strtok(NULL," ");
day=strtok(day,"=");
day=strtok(NULL," ");
printf("El dia es %s \n",day);


char path[100]={0};
strcpy(path,"aws s3 --no-sign-request ls --recursive noaa-goes16/ABI-L2-CMIPF/");
strcat(path,year);
strcat(path,"/");
strcat(path,day);
strcat(path,"/");
strcat(path," | grep M3C13");
strcat(path," >> ./informacion");
//printf("%s \n",path);

//system("aws s3 --no-sign-request ls --recursive noaa-goes16/ABI-L2-CMIPF/2017/320/ > ./info");

system("rm ./informacion");
system("touch ./informacion");
system(path);

	FILE *fp;
	int tam_file_send, n;
	char buffer[1024];

	fp=fopen("./informacion","r");
	if (fp == NULL)
	{
		printf("Error al abrir el file\n");
		exit(1);
	}

	while(!feof(fp))	//Si no llegue al final del archivo
	{	
		memset(buffer,'\0',sizeof(buffer));//Quiero mandar mas de un paquete lo reinicializo
	        fread(buffer, 1, sizeof(buffer), fp);
		printf("%s \n",buffer);
	}
	fclose(fp);	

return 0;
}


