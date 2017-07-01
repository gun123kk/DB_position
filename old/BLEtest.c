#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
int main(int argc,char *argv[]){
    int i=0;
	for(i=0;i<1000;i++){
		system("hcidump --raw");
		 sleep(1);
	}
	
	return 0;	
}
