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
printf("El listado solicitado del anio %s",year);

char *day=strtok(NULL," ");
day=strtok(day,"=");
day=strtok(NULL," ");
printf(" y dia %s es:\n",day);

char path[100]={0};
strcpy(path,"aws s3 --no-sign-request ls --recursive noaa-goes16/ABI-L2-CMIPF/");
strcat(path,year);
strcat(path,"/");

if(strlen(day)==2)
{
	char temp[15]={0};
	strcpy(temp,"0");
	strcat(temp,day);
	strcat(path,temp);
}

else if(strlen(day)==1)
{
	char temp[15]={0};
        strcpy(temp,"00");
        strcat(temp,day);
        strcat(path,temp);
}

else
{
	strcat(path,day);
}

strcat(path,"/");
strcat(path," | grep M3C13");
system(path);

return 0;

}

