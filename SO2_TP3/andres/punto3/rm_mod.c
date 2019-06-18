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

char *name=strtok(inputParse,"&");

char path[100]={0};

if(strlen(readInput)==0)
{
	printf("No ha ingresado nada \n");
}
else
{
	strcpy(path,"sudo rmmod ");
	strcat(path,name);

	system("sudo dmesg -C");
	system(path);
	system("sudo dmesg");
}
exit(0);
}

