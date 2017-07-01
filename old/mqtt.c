#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define IP

void main(int argc, char *argv[])
{
    int i=0;
   // char str[50]="mosquitto_pub -t b -m \"i\" -h \"192.168.0.18\"";
    char str1[]="mosquitto_pub -t test -m ";
    char str2[]="hi";
    char str3[]=" -h \"192.168.0.10\"";
    char str[80];
 	 strcpy(str,str1);
 	strcat(str,str2);
 	strcat(str,str3);
	printf("start\n");
  	printf("%s\n",str);
	char tmp[10];	 
	while (1)
    {
	sprintf(tmp,"%d",i);
	strcpy(str2,tmp);
	 strcpy(str,str1);
        strcat(str,str2);
        strcat(str,str3);
       

 
        system(str);
        i++;
        sleep(1);
    }
    
    exit (0);
}
