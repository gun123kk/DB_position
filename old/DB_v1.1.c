//-----------------------------------------------------------------------
//程式版本:v1.1
//修改日期:2017/06/08
//是否驗證:no
//注意事項
//v1.1新增函式庫 DBFunction.h
//gcc -pthread DB_v1.1.c DBFunction.c -lm
//-----------------------------------------------------------------------
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include "DBFunction.h"
#define choice 0	 //程式取值方式選擇 1賴捲 0俊賢
#if choice
	//賴捲
	
#else
	//俊賢
	#define RTQty	5		//取多個RSSI
	#define rssiRang	2		//RSSI 變動幅度容許範圍
#endif
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int RSSItemp[4];			
int TXPtemp[4];
int RSSI[4];			
int TXP[4];
//-----------------------------------------------------------------------
//函式宣告區
//-----------------------------------------------------------------------
void *threadReadRSSI1(void *arg);
void *threadReadRSSI2(void *arg);
void *threadReadRSSI3(void *arg);
void *threadReadRSSI4(void *arg);
void dealRSSIandTXP(int pot,char *txprsi);		//處理從檔案獲得的RSSI TXP 轉成數值
void showGetTXPandRSSI(int pot,char *txprsi);	//顯示從檔案獲得的TXP RSSI
//-------------------------
//俊賢RSSI處理函式
//-------------------------
void RSSI_BUBSort(int *sortData); //氣泡排序
int getMID(int *data);//取中位數
int chkRSSIRang(int RI,int pot); //檢查是否超出設定範圍1
//-------------------------
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//主程式
//-----------------------------------------------------------------------
int main(int argc, char *argv[])
{
	int i;
	pthread_t tRSSI1, tRSSI2 ,tRSSI3, tRSSI4;
	pthread_create(&tRSSI1, NULL, threadReadRSSI1, NULL);
	pthread_create(&tRSSI2, NULL, threadReadRSSI2, NULL);
    pthread_create(&tRSSI3, NULL, threadReadRSSI3, NULL);
	pthread_create(&tRSSI4, NULL, threadReadRSSI4, NULL);	
	sleep(3);
	
	int distance[4];
	
	while(1){
		//printf("I am main thread. i=%d\n", i);
		
		printf("TXP1:%d\n",TXP[0]);
		printf("RSSI1:%d\n",RSSI[0]);
		distance[0]=dealDistance(TXP[0],RSSI[0],0);
		printf("TXP2:%d\n",TXP[1]);
		printf("RSSI2:%d\n",RSSI[1]);
		distance[1]=dealDistance(TXP[1],RSSI[1],1);
		printf("TXP3:%d\n",TXP[2]);
		printf("RSSI3:%d\n",RSSI[2]);
		distance[2]=dealDistance(TXP[2],RSSI[2],2);
		printf("TXP4:%d\n",TXP[3]);
		printf("RSSI4:%d\n",RSSI[3]);
		distance[3]=dealDistance(TXP[3],RSSI[3],3);
		printf("------------------\n");
		//sleep(1);
		
	}	
	//pthread_mutex_destroy(&mutex);
	pthread_exit(NULL);
    exit(0);
	//return 0;
}
//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//thread
//-----------------------------------------------------------------------
//-------------------------
//threadReadRSSI1
//-------------------------
void *threadReadRSSI1(void *arg){	
	FILE *in;
	char data[7]; 	//TX RSSI
	int count=0;
	int rk=0;
	int rssi[RTQty];
	//int rssi[RTQty]={-20,-98,0,-22,-30};	//for test
	int mid;
	int start=1;
	while(1){
		in=fopen("/home/pi/iBeaconScan/BLE1.txt","r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);			
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);
#if choice
	//賴捲
#else
	//俊賢	
	rssi[count]=RSSItemp[rk];	
	count++;
	if(count==5){
		count=0;		
	}
	
	RSSI_BUBSort(rssi);	//排序
	
	mid=getMID(rssi); //中位數
	if(start){
		RSSI[rk]=mid;
		start=0;
	}
	if(chkRSSIRang(mid,rk)==1){	//幅度檢查 尚未完成
		RSSI[rk]=mid;			//大於幅度才要更改
	}	
	TXP[rk]= TXPtemp[rk];	
#endif
		fclose(in);			
	}
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadReadRSSI2
//-------------------------
void *threadReadRSSI2(void *arg){
	FILE *in;
	char data[7]; 	//TX RSSI	
	int count=0;
	int rk=1;
	int rssi[RTQty];
	//int rssi[RTQty]={-30,-108,-10,-32,-40};	//for test
	int mid;
	int start=1;
	while(1){
		in=fopen("/home/pi/iBeaconScan/BLE2.txt","r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);				
			//showGetTXPandRSSI(rk,data);				
		}	
		dealRSSIandTXP(rk,data);
#if choice	
	//賴捲	
#else
	//俊賢	
	rssi[count]=RSSItemp[rk];	
	count++;
	if(count==5){
		count=0;			
	}		
	RSSI_BUBSort(rssi);//排序
	mid=getMID(rssi);	//中位數
	if(start){
		RSSI[rk]=mid;
		start=0;
	}
	if(chkRSSIRang(mid,rk)==1){	//幅度檢查 尚未完成
		RSSI[rk]=mid;			//大於幅度才要更改
	}	
	TXP[rk]= TXPtemp[rk];	
#endif
		fclose(in);			
	}
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadReadRSSI3
//-------------------------
void *threadReadRSSI3(void *arg){
	FILE *in;
	char data[7]; 	//TX RSSI	
	int count=0;
	int rk=2;
	int rssi[RTQty];
	//int rssi[RTQty]={-87,-77,-66,-44,-33};	//for test
	int mid;
	int start=1;
	while(1){
		in=fopen("/home/pi/iBeaconScan/BLE3.txt","r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);	
#if choice
	//賴捲	
#else
	//俊賢	
	rssi[count]=RSSItemp[rk];	
	count++;
	if(count==5){
		count=0;		
	}
	RSSI_BUBSort(rssi);	//排序		
	mid=getMID(rssi);//中位數
	if(start){
		RSSI[rk]=mid;
		start=0;
	}
	if(chkRSSIRang(mid,rk)==1){	//幅度檢查 尚未完成
		RSSI[rk]=mid;			//大於幅度才要更改
	}	
	TXP[rk]= TXPtemp[rk];	
#endif
		fclose(in);			
	}
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadReadRSSI4
//-------------------------
void *threadReadRSSI4(void *arg){
	FILE *in;
	char data[7]; 	//TX RSSI	
	int count=0;
	int rk=3;
	int rssi[RTQty];
	//int rssi[RTQty]={-130,-870,-777,-543,-303};	//for test
	int mid;
	int start=1;
	while(1){
		in=fopen("/home/pi/iBeaconScan/BLE4.txt","r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);		
#if choice	
	//賴捲
#else
	//俊賢	
	rssi[count]=RSSItemp[rk];	
	count++;
	if(count==5){
		count=0;		
	}		
	RSSI_BUBSort(rssi);//排序	
	mid=getMID(rssi);//中位數
	if(start){
		RSSI[rk]=mid;
		start=0;
	}
	if(chkRSSIRang(mid,rk)==1){	//幅度檢查 尚未完成
		RSSI[rk]=mid;			//大於幅度才要更改
	}	
	TXP[rk]= TXPtemp[rk];	
#endif		
		fclose(in);			
	}
	pthread_exit(NULL);	
}
//-------------------------
//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
//函式區
//-----------------------------------------------------------------------


//-------------------------
//處理rssi 和 tx power
//-------------------------
void dealRSSIandTXP(int pot,char *txprsi){	
	//TX Power
	int txH=(txprsi[1]-48)*10;
	int txL=txprsi[2]-48;
	TXPtemp[pot]=0-(txH+txL);	
	//RSSI
	int rssiH=(txprsi[5]-48)*10;
	int rssiL=txprsi[6]-48;
	RSSItemp[pot]=0-(rssiH+rssiL);
#if 0
	printf("TX power temp%d:%d\n",pot,TXPtemp[pot]);	
	printf("RSSItemp%d:%d\n",pot,RSSItemp[pot]);		
#endif
	//sleep(1);		//1sec		
}
//-------------------------

//-------------------------
//顯示從檔案取得的RSSI和TX POWER
//-------------------------
void showGetTXPandRSSI(int pot,char *txprsi){	
	int i;
	for(i=0;i<7;i++){								 
		printf("receive[%d]:%c\n",i,txprsi[i]);
	}		 
	sleep(1);		//1sec	
}
//-------------------------

//-------------------------
//RSSI 氣泡排序
//-------------------------
void RSSI_BUBSort(int *sortData){
	int temp,i,j;
	int sc=RTQty;	
#if 0
	printf("排序前:");
	for(i=0;i<RTQty;i++){
		printf("%d ",sortData[i]);
	}
	printf("\n");
#endif		
	for(i=0;i<sc;i++){
    	for(j=0;j<sc-i-1;j++){
      		if(sortData[j]<sortData[j+1]){
        		temp = sortData[j];
      			sortData[j]=sortData[j+1];
      			sortData[j+1]=temp;
     		 }
    	}
  	}	
#if 0
	printf("排序後:");	
	for(i=0;i<RTQty;i++){
		printf("%d ",sortData[i]);
	}	
	printf("\n");
	sleep(1);
#endif	
}
//-------------------------

//-------------------------
//取中位數
//-------------------------
int getMID(int *data){
	int sc=RTQty;
	int cmid=sc/2,mid;
	mid=data[cmid];
#if 0
	printf("mid:%d\n",mid);
	sleep(1);
#endif	
	return mid;
}
//-------------------------

//-------------------------
//檢查RSSI是否超出設定範圍
//-------------------------
int chkRSSIRang(int RI,int pot){
	if(abs(RI-RSSItemp[pot])>rssiRang){
		return 1;		//超出
	}
	else{
		return 0;		//沒超出
	}
}
//-------------------------
/*
int dealDistance(int txPower,int rssi,int pot){
	
	float ratio; 
	float distance;
	ratio=rssi*1.0/txPower;
	if (ratio < 1.0) {		
		distance=pow(ratio, 10);	    
	}
	else{	  
		distance =  (0.89976)*pow(ratio, 7.7095) + 0.111;   	   
	}	
#if 1
	printf("ratio:%f\n",ratio);
	printf("distance%d:%f\n",pot,distance);
#endif

	return distance;
}
*/
//-----------------------------------------------------------------------
