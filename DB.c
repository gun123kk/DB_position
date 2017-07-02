//-----------------------------------------------------------------------
//程式版本:v2.0
//修改日期:2017/07/02
//是否驗證:yes
//注意事項
//v1.1新增函式庫 DBFunction.h
//v1.2新增函式庫 TPosition.h
//v1.3新增項目如下
//控制DC MOTOR THREAD
//控制BUZZER 和 LED THREAD
//OLED輸出 THREAD
//v1.4修改:將RSSI取得資料後處理寫成函式
//v1.5將DBFunction的距離函式移到culDistance.h
//v1.6新增如下
//1.新增THREAD threadLightSensor
//2.新增函式庫mqttPub.h MQTT Publsih函式
//3.新增THREAD threadMQTTPublish
//v1.7新增如下
//1.新增函式庫mqttSub.h MQTT Subscribe函式
//2.新增THREAD threadMQTTSubscribe
//測試ibeacon使用-20db 距離使用rssi區間判斷
//v1.8修改如下
//1.修改送出的MQTT送出的資料格式
//2.不使用距離公式直接以RSSI判斷區間
//3.OLED更改為NUM ,AREA,near ibeacan
//v1.9新增使用RSSI直接判斷九宮格位置 int rssiPosition();
//i2--------------i1
//|A1	|A2     |A3
//|A4	|A5	   |A6
//|A7	|A8	   |A9
//i4--------------i3
//v2.0將目前沒用到的函式庫刪除
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
#include "culDistance.h"
#include "mqttPub.h"
#include "mqttSub.h"

#define path1 "/root/project/position/datalog/BLE1.txt"
#define path2 "/root/project/position/datalog/BLE2.txt"
#define path3 "/root/project/position/datalog/BLE3.txt"
#define path4 "/root/project/position/datalog/BLE4.txt"
int DBnumber=1;

#define choice 0	 //程式取值方式選擇 1賴捲 0俊賢
#if choice
	//賴捲
    #define RTQty	3		//取多個RSSI
    #define rssiRang	2		//RSSI 變動幅度容許範圍
#else
	//俊賢
	#define RTQty	10	//取多個RSSI
	#define rssiRang	2		//RSSI 變動幅度容許範圍
	#define DQty	10	//取多個distance
#endif
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
int RSSItemp[4];			
int TXPtemp[4];
double RSSI[4];			
int TXP[4];
dcmotor cdcm;		//直流馬達控制
ledBuz cledBuz;  	//LED與蜂鳴器控制
oled oledInfo;		//OLED輸出
POS	posxy;				//計算出來的XY
float area[4][2]={ {0,0} , {0,3} , {3,0} , {3,3} };//1 3 2 4
double posX=0,posY=0;
int areaPos=0;
int lightSensor;		//讀取光敏電阻數值0~1024

char DCM_SW='0';
char BUZLED_SW='0';
int nearWhibeacon=0;
double distanceTemp1[DQty]={0};
double distanceTemp2[DQty]={0};
double distanceTemp3[DQty]={0};
double distanceTemp4[DQty]={0};
int rssiCount[4]={0};
int rssiCounter=0;
int canShowRSSI[4]={0};
int sureShowRSSI=0;
//-------------------------
//MQTT Publish
//-------------------------
#define MQTT_HOSTNAMEpub "192.168.0.3" 
//-------------------------

//-------------------------
//MQTT Subscribe
//-------------------------
#define MQTT_HOSTNAMEsub "192.168.0.3" 
extern char subData[5];
//-------------------------
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
void *threadLightSensor(void *arg);

void *threadMQTTPublish(void *arg);
void *threadMQTTSubscribe(void *arg);
void *threadMQTTSubscribeRead(void *arg);

void ouch(int sig);						//CRTL+C中斷函式
void dealRSSIandTXP(int pot,char *txprsi);		//處理從檔案獲得的RSSI TXP 轉成數值
void showGetTXPandRSSI(int pot,char *txprsi);	//顯示從檔案獲得的TXP RSSI
void initFunction();

int rssiPosition();
void culLongSide(double *distance);
void createDistance(double *af_distanceTP,double *bf_distance,int *count);

//-------------------------
//俊賢RSSI處理函式
//-------------------------
//void RSSI_BUBSort(int *sortData); //氣泡排序
int getMID(int *data);//取中位數
void rssiDataA(int rk,int *count,double *rssi,int *start); 
void RSSI_BUBSort(double *sortData,int rk,int all); //氣泡排序
int RSSI_BUBSort_iBeacon(double *sortData,int all); //氣泡排序
double bfRSSI[4];
//-------------------------
//-------------------------
//賴捲RSSI處理函式
//-------------------------
double avgRSSI(double *rssi , double count);//取平均
//-------------------------

//-----------------------------------------------------------------------


//-----------------------------------------------------------------------
//主程式
//-----------------------------------------------------------------------
int main(int argc, char *argv[]){	
	
	system("/root/project/position/iBeaconScanV1.9/ibeacon_scan4.sh &");	
	//system("hcitool lescan --duplicates &");
	sleep(1);
	initFunction();				//所有init	
	pthread_t tRSSI1, tRSSI2 ,tRSSI3, tRSSI4;
	pthread_t tDCM,tLEDBUZ,tOLED,tLGTSNR;
	pthread_t tMQTTPub,tMQTTSub,tMQTTSubRead;
	pthread_create(&tRSSI1, NULL, threadReadRSSI1, NULL);
	pthread_create(&tRSSI2, NULL, threadReadRSSI2, NULL);
    pthread_create(&tRSSI3, NULL, threadReadRSSI3, NULL);
	pthread_create(&tRSSI4, NULL, threadReadRSSI4, NULL);	
	pthread_create(&tDCM,NULL,threadDCMotor,NULL); 
	pthread_create(&tLEDBUZ,NULL,threadLED_Buzzer,NULL);
	pthread_create(&tOLED,NULL,threadOLED,NULL);
	pthread_create(&tLGTSNR,NULL,threadLightSensor,NULL);	
	pthread_create(&tMQTTPub,NULL,threadMQTTPublish,NULL);	
	pthread_create(&tMQTTSub,NULL,threadMQTTSubscribe,NULL);
	pthread_create(&tMQTTSubRead,NULL,threadMQTTSubscribeRead,NULL);
		
	
	double distance[4]={0,0,0,0};
	double distance1[4]={0,0,0,0};	
	double distance2[4]={0,0,0,0};	
	int count[4]={0};	
    int killsh=0;
	while(1){	
		
		//printf("I am main thread. i=%d\n", i);	
#if 1	
		/*
		printf("TXP1:%d\n",TXP[0]);
		printf("TXP2:%d\n",TXP[1]);
		printf("TXP3:%d\n",TXP[2]);
		printf("TXP4:%d\n",TXP[3]);
		*/
		printf("RSSI1:%f		",RSSI[0]);		
		printf("RSSI3:%f\n",RSSI[2]);		
		printf("RSSI2:%f		",RSSI[1]);	
		printf("RSSI4:%f\n",RSSI[3]);			
				
		printf("------------------\n");			
#endif	
		if(!sureShowRSSI){
			if((canShowRSSI[0]+canShowRSSI[1]+canShowRSSI[2]+canShowRSSI[3])==4){
				sureShowRSSI=1;
			}
		}else{
			pthread_mutex_lock(&mutex);
			areaPos=rssiPosition();
			pthread_mutex_unlock(&mutex);
			printf("areaPos:%d\n",areaPos);
		}
#if 0
		printf("distance[0]:%f\n",distance[0]);
		printf("distance[1]:%f\n",distance[1]);
		printf("distance[2]:%f\n",distance[2]);
		printf("distance[3]:%f\n",distance[3]);		
		printf("------------------\n");		
#endif	
			
#if 0
//位置測試	
		distance[0]=2.45;
		distance[1]=3.5;
		distance[2]=0.01;	
		distance[3]=2.6;	  
#endif			
	
		//TPosition(distance[0],distance[1],distance[2],distance[3],area,&posxy);	
#if 0		
		//printf("X:%f\nY:%f\n",posxy->x, posxy->y);	
		printf("X:%f\nY:%f\n",posxy.x, posxy.y);	
		printf("------------------\n");	
#endif	
	
		posX=posxy.x;
		posY=posxy.y;
#if 0		
		printf("posX:%f\nposY:%f\n",posX,posY);	
		printf("------------------\n");	
#endif		
		//usleep(1000);	
		//usleep(200000);				
		sleep(1);
		killsh++;
		//因hcidump --raw相關指令執行若無關掉持續執行，RSSI會越讀越慢，所以採10秒重開一次
		if(killsh==10){
			system("killall hcidump");
			system("killall ibeacon_scan4.sh");
			printf("----------kill-------------\n");
			system("/root/project/position/iBeaconScanV1.9/ibeacon_scan4.sh &");	
			killsh=0;
		}
	}	
	pthread_mutex_destroy(&mutex);		
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
	double rssi[RTQty];
	while(1){	
		in=fopen(path1,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);			
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);

	rssiDataA(rk,&count,rssi,&start);

		fclose(in);	
		usleep(500000);
		//usleep(100000);
		//sleep(1);		
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
	double rssi[RTQty];
	while(1){		
		in=fopen(path2,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);				
			//showGetTXPandRSSI(rk,data);				
		}	
		dealRSSIandTXP(rk,data);

	rssiDataA(rk,&count,rssi,&start);	

		fclose(in);	
		usleep(500000);
		//usleep(100000);
		//sleep(1);
		
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
	double rssi[RTQty];
	while(1){		
		in=fopen(path3,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);	

	rssiDataA(rk,&count,rssi,&start);	
		
		fclose(in);	
		usleep(500000);	
		//usleep(100000);
			//sleep(1);
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
	double rssi[RTQty];
	
	while(1){	
		in=fopen(path4,"r");
		if(!in){
			perror("fopen");			
		}else{			
			fread(data,sizeof(data),1,in);	
			//showGetTXPandRSSI(rk,data);			
		}
		dealRSSIandTXP(rk,data);				 
	
	rssiDataA(rk,&count,rssi,&start);	
	
		fclose(in);	
		usleep(500000);	
		//usleep(100000);
		//sleep(1);
	}	
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadDCMotor
//-------------------------
void *threadDCMotor(void *arg){	

	while(1){
#if 0
	BtnColDCM();			//按鈕控制
#else
	//cdcm.onOff=由接收信號控制
	cdcm.delayTime=3;	//sec
	if(DCM_SW=='1'){	
		pthread_mutex_lock(&mutex);
		subData[2]='0';
		pthread_mutex_unlock(&mutex);
		cdcm.onOff=1;		
		DCMotorONOFFwithDT(&cdcm);			
		cdcm.onOff=0;
		pthread_mutex_lock(&mutex);
		DCM_SW=='0';
		subData[0]='0';
		pthread_mutex_unlock(&mutex);
		
	}
#endif		
	}
	pthread_exit(NULL);	
}
//-------------------------

//-------------------------
//threadLED_Buzzer
//-------------------------
void *threadLED_Buzzer(void *arg){
	while(1){
#if  0
	BtnColLED_Buzzer();			//按鈕控制
#else	
	//cledBuzonOff=由接收信號控制
	cledBuz.count=3;	//次數	
	if(BUZLED_SW=='1'){
		pthread_mutex_lock(&mutex);
		subData[4]='0';
		pthread_mutex_unlock(&mutex);
		cledBuz.onOff=1;	//信號控制		
		LED_BUZ_ONOFFwithCT(&cledBuz);		
		cledBuz.onOff=0;	//信號控制	
		pthread_mutex_lock(&mutex);
		BUZLED_SW=='0';
		subData[0]='0';	
		pthread_mutex_unlock(&mutex);
		
	}			
#endif					
	}
	pthread_exit(NULL);
}
//-------------------------

//-------------------------
//threadOLED
//-------------------------
void *threadOLED(void *arg){
	oledInfo.number=DBnumber;
	oledInfo.area=0;
	oledInfo.near=nearWhibeacon;
	dealOLEDData(&oledInfo);
	while(1){	
		oledInfo.area=0;		
		oledInfo.near=nearWhibeacon;
		dealOLEDData(&oledInfo);
		usleep(50000);			
	}	
	pthread_exit(NULL);
}
//-------------------------

//-------------------------
//threadLightSensor
//-------------------------
void *threadLightSensor(void *arg){
	while(1){		
		lightSensor=getLightSensorVal();		
#if 0
		printf("lightSensr:%d\n",lightSensor);	
#endif
	}	
	pthread_exit(NULL);
}
//-------------------------

//-------------------------
//threadMQTTPublish
//-------------------------
void *threadMQTTPublish(void *arg){
	while(1){		
		int i;
		/*
		for(i=0;i<10;i++){
			publishDBData(DBnumber,areaPos,lightSensor);	
			areaPos=i;
			usleep(500000);
		}*/	
		
		publishDBData(DBnumber,areaPos,lightSensor);
		//publishData("type your data");		//fortest can publsih data to mqtt server
		//sleep(1);
		usleep(250000);
	}
	pthread_exit(NULL);
}
//-------------------------

//-------------------------
//threadMQTTSubscribe
//-------------------------
void *threadMQTTSubscribe(void *arg){
	while(1){		
		subscribehtmlData();	
	}
	pthread_exit(NULL);
}
//-------------------------


//-------------------------
//threadMQTTSubscribeRead
//-------------------------
void *threadMQTTSubscribeRead(void *arg){
	while(1){	
		//1,1,1 代表num1 1開啟DCMOTOR 1開啟BUZZER+LED
		//1,0,0 代表num1 0關閉DCMOTOR 0關閉BUZZER+LED		
		
		if(subData[0]==DBnumber+'0'){//+'0' chang to char		
			
			if(subData[2]=='1'){
					pthread_mutex_lock(&mutex);
					DCM_SW='1';		
					pthread_mutex_unlock(&mutex);
					//printf ("subData[2]=1\n");						
			}else{
					pthread_mutex_lock(&mutex);
					DCM_SW='0';	
					pthread_mutex_unlock(&mutex);	
					//printf ("subData[2]=0\n");
			}
			if(subData[4]=='1'){	
					pthread_mutex_lock(&mutex);
					BUZLED_SW='1';	
					pthread_mutex_unlock(&mutex);		
					//printf ("subData[4]=1\n");
			}else{
					pthread_mutex_lock(&mutex);
					BUZLED_SW='0';	
					pthread_mutex_unlock(&mutex);
					//printf ("subData[4]=0\n");
			}
		}
	}
	pthread_exit(NULL);
}
//-------------------------

//-----------------------------------------------------------------------




//-----------------------------------------------------------------------
//函式區
//-----------------------------------------------------------------------
//-------------------------
//初始化
//-------------------------
void initFunction(){
	DBF_init();	
	mqttsubInit(MQTT_HOSTNAMEsub);
	sleep(1);
	mqttpubInit(MQTT_HOSTNAMEpub);		//mqtt publish
	
	signal(SIGINT,ouch);	//系統中斷	
}
//-------------------------

//-------------------------
//系統中斷
//-------------------------
void ouch(int sig){
	DBF_close();
	mqttpubClose();
	mqttsubClose();
	bcm2835_close();
}
//------------------------

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
void RSSI_BUBSort(double *sortData,int rk,int all){
	int i,j;
	double temp;
	//int sc=RTQty;	
	int sc=all;
	int minor=rk+1;
#if 0
	printf("minor:%d排序前:",minor);
	for(i=0;i<sc;i++){
		printf("%f ",sortData[i]);
	}	
	printf("\n");
	printf("-----------------------------\n");
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
	//if(rk==1){
		printf("minor:%d排序後:",minor);	
		for(i=0;i<sc;i++){
			printf("%f ",sortData[i]);
		}		
		printf("\n");
		printf("-----------------------------\n");
	//}
#endif	
}
//-------------------------

//-------------------------
//RSSI 氣泡排序 找龍珠最靠近哪個IBEACON
//-------------------------
int RSSI_BUBSort_iBeacon(double *sortData,int all){ 
	int i,j;
	double temp;
	int tempIBN;
	int sc=all;	
	int ibeacon[4]={0,1,2,3};
#if 0	
	for(i=0;i<sc;i++){
		printf("%f ",sortData[i]);
		
	printf("\n");
	printf("-----------------------------\n");
	}
#endif		
	for(i=0;i<sc;i++){
    	for(j=0;j<sc-i-1;j++){
      		if(sortData[j]<sortData[j+1]){
								
        		temp = sortData[j];				
      			sortData[j]=sortData[j+1];
      			sortData[j+1]=temp;
				
				tempIBN=ibeacon[j];
				ibeacon[j]=ibeacon[j+1];
      			ibeacon[j+1]=tempIBN;
     		 }
    	}
  	}	
#if 0	
	for(i=0;i<sc;i++){
		printf("%f ",sortData[i]);
			
	printf("\n");
	printf("-----------------------------\n");
	}
#endif	

	return ibeacon[0];
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
//RSSI取後處理
//-------------------------
void rssiDataA(int rk,int *count,double *rssi,int *start){
	int mid;
	double avg=0;
	int cc=*count;
	
	//RSSI[rk]=RSSItemp[rk];	
	//printf("RSSItemp[%d]:%d\n",rk,RSSItemp[rk]);
	
	if(abs(RSSItemp[rk])>10 && RSSItemp[rk]>=-85){
		rssi[cc]=RSSItemp[rk];		
		cc++;
		*count=cc;
		//printf("count:%d\n",*count);
	}	
	
	if(cc==RTQty){	
		canShowRSSI[rk]=1;
		cc=0;
		*count=cc;
		//avg = avgRSSI(rssi , RTQty); //RSSI取平均
		//RSSI[rk]=avg;		
		RSSI_BUBSort(rssi,rk,RTQty);	//排序	
		//int num=10;
		int i=0;
		//int count[4]={0};		
		//排序後的最小最大值不列入平均
		for(i=2;i<6;i++){
			avg=avg+rssi[i];
		}
		avg=avg/4;
		RSSI[rk]=avg;
		
		if(*start){
			bfRSSI[rk]=avg;		
			*start=0;
		}else{			
			//檢查是否大於設定範圍
			if(abs(avg-rssiRang)>bfRSSI[rk]){				
				RSSI[rk]=avg;
				bfRSSI[rk]=avg;	
			}else{
				RSSI[rk]=bfRSSI[rk];
			}
		}
	}	
	/*
	if(rk==1){
		printf("//------------------------\n");
		printf("AVG RSSI[%d]:%f\n",rk,RSSI[rk]);
		printf("//------------------------\n");		
	}*/
	TXP[rk]= TXPtemp[rk];
}
//-------------------------


//-------------------------
//RSSI判斷位置處理
//-------------------------
int  rssiPosition(){
	
	int rssiHILOWCheck[4]={0};
	int pos=0;
	double rssiTP[4];
	int i;
	int max1=0;
	int max2=0;
	int min=0;	
	int range1=-58;
	int range2=-70;
	int range3=-80;
	int near_ibeacon=0;
	int check=0;
	int near=0;
	int breakNear=0;
	int centerRange=12;
			/*
			if(abs(RSSI[1]-5) <82){
				RSSI[1]=RSSI[1]-5;//for test ibeacon2 RSSI值偏小 加上OFFEST
			}*/
	
		for(i=0;i<4;i++){
			rssiTP[i]=RSSI[i];
		}		
		RSSI_BUBSort(rssiTP,0,4);
		
		for(i=0;i<4;i++){
			printf("rssiTP[%d]:%f\n",i,rssiTP[i]);			
			if(rssiTP[0]==RSSI[i]){		//[0]代表離ibeacon最近 RSSI最大
				max1=i+1;	
				printf("max1:%d\n",max1);		
			}	
			if(rssiTP[1]==RSSI[i]){		//[0]代表離ibeacon最近 RSSI第二大
				max2=i+1;			
				printf("max2:%d\n",max2);		
			}
			
			if(rssiTP[3]==RSSI[i]){		//[3]代表離ibeacon最遠 RSSI最小
				min=i+1;
			}				
		}	
		
		if(RSSI[max1]>range1){
				//---------------------------------
				//再次確認是否龍珠真的最靠近ibeacon
				switch(max1){
					case 0:
						//檢查ibeacon4的對角
						if(RSSI[3]>=range3 ){
							breakNear=1;
						}
					break;
					
					case 1:
						//檢查ibeacon2的對角
						if(RSSI[2]>=range3 ){
							printf("IN\n");
							breakNear=1;
						}
					break;
					
					case 2:
						//檢查ibeacon3的對角
						if(RSSI[1]>=range3 ){
							breakNear=1;
						}
					break;
					
					case 3:
						//檢查ibeacon4的對角				
						if(RSSI[0]>=range3){
							breakNear=1;							
						}
					break;			
				}
		}

		//---------------------------------
		/*
		//--------------------------
		//眾數		
		rssiCount[max]=rssiCount[max]+1;
		rssiCounter++;
		if(rssiCounter==10){				
			near_ibeacon=RSSI_BUBSort_iBeacon(rssiTP,4);			
			nearWhibeacon=near_ibeacon+1;
			rssiCounter=0;
		}
		//--------------------------
			*/	
			
			if(!breakNear){
				//--------------------------
				//判斷龍珠誰靠近IBEACON 有沒有重複
				for(i=0;i<4;i++){
					if(RSSI[i]>=range1){
						rssiHILOWCheck[i]=1;
						near=1;				
					}else{
						rssiHILOWCheck[i]=0;	
					}
					check=check+rssiHILOWCheck[i];			
				}
				//--------------------------
			}else{
				near=0;
			}
			
				if(near==1){						
					nearWhibeacon=max1;
					//printf("max:%d\n",max1);
					if(check==1){
						//離IBEACON最近
						if(max1==1){
							pos=3;
						}else if(max1==2){
							pos=1;	
						}else if(max1==3){
							pos=9;	
						}else if(max1==4){
							pos=7;	
						}else{
							pos=0;
						}
					}else{
						//有兩個都大於-RSSI70
						if(min==4){
							pos=3;
						}else if(min==3){
							pos=1;	
						}else if(min==2){
							pos=9;	
						}else if(min==1){
							pos=7;	
						}else{
							pos=0;
						}
					}
				}else{
					nearWhibeacon=0;
					int count=0;
					for(i=0;i<4;i++){
						if(RSSI[i]<range2){
							count++;
						}							
					}
					if(count>3){
						pos=5;
						count=0;
					}else{
							//是否在區間2				
							if((max1==1 && max2==2) || (max1==2 && max2==1 )){						
								pos=2;
							}
							//是否在區間4				
							if((max1==2 && max2==4) || (max1==4 && max2==2 )){						
								pos=4;
							}
							//是否在區間6			
							if((max1==1 && max2==3) || (max1==3 && max2==1 )){						
								pos=6;
							}
							//是否在區間8			
							if((max1==3 && max2==4) || (max1==4 && max2==3 )){						
								pos=8;
							}		
							if(!(pos>=2 && pos<=8)){
								pos=5;
							}							
					}			
				}			
		
	return pos;
}
//-------------------------

void culLongSide(double *distance){
	/*
		float longSide=2.60;
		float shortSide=2.45;
		float limit1=3;	
		float limit2=2.87;	
		float limit3=2.77;	
		*/
		float longSide=3;
		float shortSide=3;
		float limit1=4;			
		int Check1=0;
		float nearD=0.5;
		double distanceBT[4];
		int i;
		for(i=0;i<4;i++){
			distanceBT[i]=distance[i];
		}
		
		RSSI_BUBSort(distanceBT,0,4);
		int cc=0;
		for(i=0;i<4;i++){
			//printf("distanceBT[%d]:%f\n",i,distanceBT[i]);
			if(distanceBT[3]==distance[i]){
				cc=i;
			}			
		}		
		
		
		if(cc==0 &&distance[0]<=nearD){		
			//if(distance[1]>=shortSide-0.5 && distance[2]>=shortSide-0.5){
				//DB near ibeacon1
				//ibeacon2-DB>=longSide ibeacon3-DB>=shortSide 
				nearWhibeacon=1;
				distance[1]=longSide;
				distance[2]=shortSide;
				distance[3]=limit1;	
				printf("near ibeacon1\n");
				Check1=1;				
			//}			
		}else if(cc==1 &&distance[1]<=nearD){			
			if(distance[0]>=shortSide-0.5 && distance[3]>=shortSide-0.5){
				//DB near ibeacon2
				//ibeacon1-DB>=longSide ibeacon4-DB>=shortSide 
				nearWhibeacon=2;
				distance[0]=longSide;
				distance[3]=shortSide;
				distance[2]=limit1;	
				printf("near ibeacon2\n");
				Check1=1;				
			}
		}else if( cc==2 &&distance[2]<=nearD){
			//if(distance[0]>=longSide-0.5 && distance[3]>=shortSide-0.5){
				//DB near ibeacon3
				//ibeacon1-DB>=shortSide ibeacon4-DB>=longSide 	
				nearWhibeacon=3;
				distance[0]=shortSide;
				distance[3]=longSide;
				distance[1]=limit1;	
				printf("near ibeacon3\n");	
				Check1=1;	
			//}
		}else if(cc==3 && distance[3]<=nearD){
			//if(distance[1]>=longSide-0.5 && distance[2]>=shortSide-0.5){
				//DB near ibeacon4
				nearWhibeacon=4;
				//ibeacon2-DB>=shortSide ibeacon3-DB>=longSide 				
				distance[1]=longSide;
				distance[2]=shortSide;	
				distance[0]=limit1;
				printf("near ibeacon4\n");	
				Check1=1;	
			//}
		}else{
			nearWhibeacon=0;
		}
		/*
		float EQ1324=2.77;
		float EQ1234=2.87;
		if(!Check1){
			if(distance[0]==distance[1]&& distance[0]<shortSide-0.5 && distance[1]<shortSide-0.5){
				//1==2
				distance[2]=EQ1234;
				distance[3]=EQ1234;
			}else if(distance[0]==distance[2] && distance[0]<longSide-0.5 && distance[2]<longSide-0.5){
				//1==3
				distance[1]=EQ1324;
				distance[3]=EQ1324;
			}else if(distance[1]==distance[3] && distance[0]<longSide-0.5 && distance[2]<longSide-0.5){
				//2==4
				distance[0]=EQ1324;
				distance[2]= EQ1324;
			}else if(distance[2]==distance[3] && distance[2]<shortSide-0.5 && distance[3]<shortSide-0.5){
				//3==4
				distance[0]=EQ1234;
				distance[1]=EQ1234;
			}
		}	*/
		
}


void createDistance(double *af_distance,double *bf_distance,int *count){
		int i=0,j=0;
		int check[4]={0,0,0,0};
		for(i=0;i<4;i++){
			if(RSSI[i]>=-46){
				bf_distance[i]=0;
			}else if(RSSI[i]<=-47 && RSSI[i]>-65){				
				bf_distance[i]=0.1;//0.1m
			}else if(RSSI[i]<=-65 && RSSI[i]>-71){				
				bf_distance[i]=0.25;//0.25m
			}else if(RSSI[i]<=-71 && RSSI[i]>-76){				
				bf_distance[i]=0.5;//0.5m
			}else if(RSSI[i]<=-76 && RSSI[i]>-82){				
				bf_distance[i]=1.0;//1m
			}else if(RSSI[i]<=-82 && RSSI[i]>-86){				
				bf_distance[i]=1.5;//1.5m
			}else if(RSSI[i]<=-86 &&   RSSI[i]>-93){			
				bf_distance[i]=3.0;	//3m			
			}else if(RSSI[i]<=-93){			
				bf_distance[i]=3.5;	//3.5m			
			}
				int cc;
				if(i==0){
					cc=count[i];
					distanceTemp1[cc]=bf_distance[i];	
					count[i]=count[i]+1;
					if(count[i]==DQty){
						count[i]=0;					
					}
				}
				if(i==1){
					cc=count[i];
					distanceTemp2[cc]=bf_distance[i];	
					count[i]=count[i]+1;
					if(count[i]==DQty){
						count[i]=0;				
					}
				}
				if(i==2){
					cc=count[i];
					distanceTemp3[cc]=bf_distance[i];	
					count[i]=count[i]+1;
					if(count[i]==DQty){
						count[i]=0;						
					}
				}
				if(i==3){
					cc=count[i];
					distanceTemp4[cc]=bf_distance[i];	
					count[i]=count[i]+1;
					if(count[i]==DQty){
						count[i]=0;						
					}
				}		
		}
		
		
		//sort
		if(count[0]==0){
			RSSI_BUBSort(distanceTemp1,0,DQty);
			check[0]=1;
		}
		if(count[1]==0){
			RSSI_BUBSort(distanceTemp2,1,DQty);
			check[1]=1;
		}
		if(count[2]==0){
			RSSI_BUBSort(distanceTemp3,2,DQty);
			check[2]=1;
		}
		if(count[2]==0){
			RSSI_BUBSort(distanceTemp4,3,DQty);
			check[3]=1;
		}
		double davg;
		if(check[0] && check[1] && check[2] && check[3]){
			for(i=0;i<4;i++){
				for(j=4;j<DQty-2;j++){
					if(i==0){
						davg=davg+distanceTemp1[j];
					}else if(i==1){
						davg=davg+distanceTemp2[j];
					}else if(i==2){
						davg=davg+distanceTemp3[j];
					}else if(i==3){
						davg=davg+distanceTemp4[j];
					}
					if(j==DQty-3){
						af_distance[i]=davg/4;
						davg=0;
						check[i]=0;
					}
				}			
			}
			culLongSide(af_distance);
		}
		
		
}
//-----------------------------------------------------------------------
