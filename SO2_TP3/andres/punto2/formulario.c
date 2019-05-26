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

char * day=strtok(dia,"=");
day=strtok(NULL," ");

char *year=strtok(anio,"&");
printf("El dia es %s \n",day);
printf("El anio es %s \n",year);

char path[100]={0};
strcpy(path,"aws s3 --no-sign-request ls --recursive noaa-goes16/ABI-L2-CMIPF/");
strcat(path,year);
strcat(path,"/");
strcat(path,day);
strcat(path,"/");
strcat(path," | grep M3C13");
strcat(path," >> ./informacion");
printf("%s \n",path);
system("aws s3 --no-sign-request ls --recursive noaa-goes16/ABI-L2-CMIPF/2017/320/ >> ./info");
printf("HOLIS \n");
return 0;

}
