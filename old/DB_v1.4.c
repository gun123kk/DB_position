//-----------------------------------------------------------------------
//程式版本:v1.4
//修改日期:2017/06/16
//是否驗證:no
//注意事項
//v1.1新增函式庫 DBFunction.h
//v1.2新增函式庫 TPosition.h
//gcc -pthread DB_v1.2.c DBFunction.c TPosition.c -lm
//目前考慮須將TXPower寫死-76,因四月固定為-59,這樣距離算不準
//v1.3新增項目如下
//控制DC MOTOR THREAD
//控制BUZZER 和 LED THREAD
//OLED輸出 THREAD
//v1.4修改如下
//1.將RSSI取得資料後處理寫成函式
//-----------------------------------------------------------------------
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>
#include <math.h>
#include "DBFunction.h"
#include <bcm2835.h>
#include "TPosition.h"
#include <signal.h>

#define path1 "/root/project/position/datalog/BLE1.txt"
#define path2 "/root/project/position/datalog/BLE2.txt"
#define path3 "/root/project/position/datalog/BLE3.txt"
#define path4 "/root/project/position/datalog/BLE4.txt"
#define choice 0	 //程式取值方式選擇 1賴捲 0俊賢
#if choice
	//賴捲
    #define RTQty	50		//取多個RSSI
    #define rssiRang	2		//RSSI 變動幅度容許範圍
#else
	//俊賢
	#define RTQty	10	//取多個RSSI
	#define rssiRang	3		//RSSI 變動幅度容許範圍
#endif
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int RSSItemp[4];			
int TXPtemp[4];
double RSSI[4];			
int TXP[4];
dcmotor cdcm;		//直流馬達控制
ledBuz cledBuz;  	//LED與蜂鳴器控制
oled oledInfo;		//OLED輸出
POS	*posxy;				//計算出來的XY
float areaX=6.9,areaY=4.45;
double posX,posY;
//-----------------------------------------------------------------------
//函式宣告區
//-----------------------------------------------------------------------
void *threadReadRSSI1(void *arg);
void *threadReadRSSI2(void *arg);
void *threadReadRSSI3(void *arg);
void *threadReadRSSI4(void *arg);
void *threadDCMotor(void *arg);
void *threadLED_Buzzer(void *arg);
void *threadOLED(void *arg);
void ouch(int sig);						//CRTL+C中斷函式
void dealRSSIandTXP(int pot,char *txprsi);		//處理從檔案獲得的RSSI TXP 轉成數值
void showGetTXPandRSSI(int pot,char *txprsi);	//顯示從檔案獲得的TXP RSSI
//-------------------------
//俊賢RSSI處理函式
//-------------------------
void RSSI_BUBSort(int *sortData); //氣泡排序
int getMID(int *data);//取中位數
int chkRSSIRang(int RI,int pot); //檢查是否超出設定範圍1
void rssiDataA(int rk,int *count,double *rssi,int *start); 
//-------------------------
//賴捲RSSI處理函式
//-------------------------
double avgRSSI(double *rssi , double count);//取平均
void rssiDataB(int rk,int *count,double *rssi); 
//-------------------------

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//主程式
//-----------------------------------------------------------------------
int main(int argc, char *argv[])
{
	
	int i;
	DBF_init();	
	signal(SIGINT,ouch);
	pthread_t tRSSI1, tRSSI2 ,tRSSI3, tRSSI4;
	pthread_t tDCM,tLEDBUZ,tOLED;	
	pthread_create(&tRSSI1, NULL, threadReadRSSI1, NULL);
	pthread_create(&tRSSI2, NULL, threadReadRSSI2, NULL);
    pthread_create(&tRSSI3, NULL, threadReadRSSI3, NULL);
	pthread_create(&tRSSI4, NULL, threadReadRSSI4, NULL);	
	pthread_create(&tDCM,NULL,threadDCMotor,NULL); 
	pthread_create(&tLEDBUZ,NULL,threadLED_Buzzer,NULL);
	pthread_create(&tOLED,NULL,threadOLED,NULL);
	sleep(1);	
	double distance[4];	
	while(1){
		//printf("I am main thread. i=%d\n", i);	
			/*
		printf("TXP1:%d\n",TXP[0]);
		printf("RSSI1:%f\n",RSSI[0]);		
		printf("TXP2:%d\n",TXP[1]);
		printf("RSSI2:%f\n",RSSI[1]);		
		printf("TXP3:%d\n",TXP[2]);
		printf("RSSI3:%f\n",RSSI[2]);			
		printf("-TXP4:%d\n",TXP[3]);
		printf("-RSSI4:%f\n",RSSI[3]);		
		*/		
		/*
		distance[0]=dealDistance(TXP[0],RSSI[0],0);
		distance[1]=dealDistance(TXP[1],RSSI[1],1);
		distance[2]=dealDistance(TXP[2],RSSI[2],2);
		distance[3]=dealDistance(TXP[3],RSSI[3],3);
		*//*
		printf("distance[0]:%f\n",distance[0]);
		printf("distance[1]:%f\n",distance[1]);
		printf("distance[2]:%f\n",distance[2]);
		printf("distance[3]:%f\n",distance[3]);
		*/
		//posX=distance[3];		
#if 0
//距離測試
/*
      distance[0]=sqrt(pow(15, 2)+pow(17, 2));
      distance[1]=sqrt(pow(15, 2)+pow(3, 2));
      distance[2]=sqrt(pow(15, 2)+pow(17, 2));
      distance[3]=sqrt(pow(15, 2)+pow(3, 2));	  
	  */
#endif
		distance[0]=4.45;
		distance[1]=0;
		distance[2]=8.18;	
		distance[3]=6.9;	
		posxy=TPosition(distance[0],distance[1],distance[2],distance[3],areaX,areaY);
		//printf("------------------\n");		
		//printf("X:%f\nY:%f\n",posxy->x, posxy->y);	
		posX=posxy->x;
		posY=posxy->y;		
		printf("posX:%f\nposY:%f\n",posX,posY);	
		sleep(1);	
		
	}	
	//pthread_mutex_destroy(&mutex);		
    exit(0);
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
	int start=1;
	while(1){	
		in=fopen(path1,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);			
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);
#if choice
	//賴捲
	double rssi[RTQty];
	rssiDataB(rk,&count,rssi);

#else
	//俊賢
	double rssi[RTQty];
	rssiDataA(rk,&count,rssi,&start);
#endif
		fclose(in);	
		usleep(500000);	
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
	int start=1;
	while(1){		
		in=fopen(path2,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);				
			//showGetTXPandRSSI(rk,data);				
		}	
		dealRSSIandTXP(rk,data);
#if choice	
	//賴捲
	double rssi[RTQty];
	rssiDataB(rk,&count,rssi);
#else
	//俊賢	
	double rssi[RTQty];
	rssiDataA(rk,&count,rssi,&start);	
#endif
		fclose(in);	
		usleep(500000);	
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
	int start=1;
	while(1){		
		in=fopen(path3,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);	
#if choice
	//賴捲
	double rssi[RTQty];
	rssiDataB(rk,&count,rssi);
#else
	//俊賢
	double rssi[RTQty];
	rssiDataA(rk,&count,rssi,&start);	
#endif		
		fclose(in);	
		usleep(500000);			
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
	int start=1;
	while(1){	
		in=fopen(path4,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);		
#if choice	
	//賴捲
	double rssi[RTQty];
	rssiDataB(rk,&count,rssi);
#else
	//俊賢	
	//int rssi[RTQty];	
	double rssi[RTQty];
	rssiDataA(rk,&count,rssi,&start);	
#endif		
		fclose(in);	
		usleep(500000);	
		//posY=RSSI[rk];		//for test
	}	
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadDCMotor
//-------------------------
void *threadDCMotor(void *arg){	
	while(1){
#if 1
	BtnColDCM();			//按鈕控制
#else
	//cdcm.onOff=由接收信號控制
	cdcm.delayTime=10;	//sec
	cdcm.onOff=1;	
	DCMotorONOFFwithDT(&cdcm);	
#endif	 
	}
	DBF_close();
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadLED_Buzzer
//-------------------------
void *threadLED_Buzzer(void *arg){
	while(1){
#if  1
	BtnColLED_Buzzer();			//按鈕控制
#else	
	//cledBuzonOff=由接收信號控制
	cledBuz.count=3;	//次數
	cledBuz.onOff=1;	//信號控制
	LED_BUZ_ONOFFwithCT(&cledBuz);	
	
#endif					
	}
	DBF_close();
	pthread_exit(NULL);
}
//-------------------------

//-------------------------
//threadOLED
//-------------------------
void *threadOLED(void *arg){
	oledInfo.number=1;
	oledInfo.posX=0;
	oledInfo.posY=0;
	dealOLEDDate(&oledInfo);
	while(1){
	//oledInfo.posX=計算後的數值;
	//oledInfo.posY=計算後的數值;
	oledInfo.number=1;	
	oledInfo.posX=posX*100;		
	oledInfo.posY=posY*100;
		dealOLEDDate(&oledInfo);
		sleep(1);
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
	//sleep(1);		//1sec	
}
//-------------------------

//-------------------------
//RSSI 氣泡排序
//-------------------------
void RSSI_BUBSort(int *sortData){
	int i,j;
	int temp;
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
#endif	
}
//-------------------------

//-------------------------
//取中位數
//-------------------------
int getMID(int *data){
	int sc=RTQty;
	int cmid=sc/2;
	int mid;
	mid=data[cmid];
#if 0
	printf("mid:%d\n",mid);
#endif	
	return mid;
}
//-------------------------

//-------------------------
//取平均
//-------------------------
double avgRSSI(double *rssi , double count)
{
    int i;
	double RSSISUM=0, avg;
    
    for(i=0; i < RTQty; i++)
        RSSISUM += rssi[i];
    
    avg = RSSISUM / RTQty;
    
    return avg;
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

//-------------------------
//系統中斷
//-------------------------
void ouch(int sig){
	DBF_close();
	bcm2835_close();
}
//------------------------

//-------------------------
//俊賢RSSI取後處理
//-------------------------
void rssiDataA(int rk,int *count,double *rssi,int *start){
	int mid;
	double avg;
	//printf("RSSItemp[%d]:%d\n",rk,RSSItemp[rk]);
	if(abs(RSSItemp[rk])>0){
		rssi[*count]=RSSItemp[rk];	
		*count=*count+1;
		//printf("count:%d\n",*count);
	}	
	if(*count==RTQty){		
		*count=0;
		avg = avgRSSI(rssi , RTQty); //RSSI取平均
		RSSI[rk]=avg;
			/*
		RSSI_BUBSort(rssi);	//排序	
		mid=getMID(rssi); //中位數
		
		if(*start){
			RSSI[rk]=mid;
			*start=0;
		}
		if(chkRSSIRang(mid,rk)==1){	//幅度檢查 
			RSSI[rk]=mid;			//大於幅度才要更改			
		}*/
	}	
	//printf("--RSSI[%d]:%d\n",rk,RSSI[rk]);		
	TXP[rk]= TXPtemp[rk];
}
//-------------------------

//-------------------------
//賴捲RSSI取後處理
//-------------------------
void rssiDataB(int rk,int *count,double *rssi){
    double avg;
    if( RSSItemp[rk] != 0 )
    {
        rssi[*count]=RSSItemp[rk];
        *count=*count+1;
        *count = *count % RTQty;
        //printf("%f %f\n", *count, rssi[count] );        
        avg = avgRSSI(rssi , RTQty); //RSSI取平均
        //printf("avgRSSI: %f\n", avg);
        RSSI[rk] = avg;
		//printf("RSSI[%d]:%f\n",rk, avg);
        TXP[rk]= TXPtemp[rk];
		
    }
}
//-------------------------

//-----------------------------------------------------------------------
