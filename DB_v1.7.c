//-----------------------------------------------------------------------
//程式版本:v1.7
//修改日期:2017/06/19
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
int DBnumber=3;

#define choice 0	 //程式取值方式選擇 1賴捲 0俊賢
#if choice
	//賴捲
    #define RTQty	3		//取多個RSSI
    #define rssiRang	2		//RSSI 變動幅度容許範圍
#else
	//俊賢
	#define RTQty	6	//取多個RSSI
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
//POS	*posxy;				//計算出來的XY
POS	posxy;				//計算出來的XY
float areaX=3,areaY=3;
double posX=0,posY=0;
int lightSensor;		//讀取光敏電阻數值0~1024

char DCM_SW='0';
char BUZLED_SW='0';
int pauseOLED=0;
//-------------------------
//MQTT Publish
//-------------------------
#define MQTT_HOSTNAMEpub "192.168.6.22" 
//-------------------------

//-------------------------
//MQTT Subscribe
//-------------------------
#define MQTT_HOSTNAMEsub "192.168.6.22" 
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
//-------------------------
//俊賢RSSI處理函式
//-------------------------
//void RSSI_BUBSort(int *sortData); //氣泡排序
int getMID(int *data);//取中位數
void rssiDataA(int rk,int *count,double *rssi,int *start); 
void RSSI_BUBSort(double *sortData,int rk); //氣泡排序
double bfRSSI[4];
//-------------------------
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
int main(int argc, char *argv[]){	
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
	sleep(1);	
	double distance[4];	
	int count=0;

	double testPOSx[5]={0,1,1.5,2,2.5};
	double testPOSy[5]={0,1,1.5,2,2.5};
	while(1){	
		
		//printf("I am main thread. i=%d\n", i);	
#if 0	
		printf("TXP1:%d\n",TXP[0]);
		printf("RSSI1:%f\n",RSSI[0]);		
		printf("TXP2:%d\n",TXP[1]);
		printf("RSSI2:%f\n",RSSI[1]);		
		printf("TXP3:%d\n",TXP[2]);
		printf("RSSI3:%f\n",RSSI[2]);			
		printf("TXP4:%d\n",TXP[3]);
		printf("RSSI4:%f\n",RSSI[3]);		
		printf("------------------\n");			
#endif	
		distance[0]=dealDistance(TXP[0],RSSI[0],0);
		distance[1]=dealDistance(TXP[1],RSSI[1],1);
		distance[2]=dealDistance(TXP[2],RSSI[2],2);
		distance[3]=dealDistance(TXP[3],RSSI[3],3);			
		/*
		distance[0]=dealDistance_DISoffSet(TXP[0],RSSI[0],0,0.5);
		distance[1]=dealDistance_DISoffSet(TXP[1],RSSI[1],1,0.5);
		distance[2]=dealDistance_DISoffSet(TXP[2],RSSI[2],2,0.5);
		distance[3]=dealDistance_DISoffSet(TXP[3],RSSI[3],3,0.5);
		*/		
#if 0
		printf("distance[0]:%f\n",distance[0]);
		printf("distance[1]:%f\n",distance[1]);
		printf("distance[2]:%f\n",distance[2]);
		printf("distance[3]:%f\n",distance[3]);		
		printf("------------------\n");		
#endif	
			
#if 0
//位置測試
		distance[0]=sqrt(pow(15, 2)+pow(17, 2));
		distance[1]=sqrt(pow(15, 2)+pow(3, 2));
		distance[2]=sqrt(pow(15, 2)+pow(17, 2));
		distance[3]=sqrt(pow(15, 2)+pow(3, 2));	  
		distance[0]=4.45;
		distance[1]=0;
		distance[2]=8.18;	
		distance[3]=6.9;	  
#endif			
		//posxy=TPosition(distance[0],distance[1],distance[2],distance[3],areaX,areaY);	
		TPosition(distance[0],distance[1],distance[2],distance[3],areaX,areaY,&posxy);	
#if 0		
		//printf("X:%f\nY:%f\n",posxy->x, posxy->y);	
		printf("X:%f\nY:%f\n",posxy.x, posxy.y);	
		printf("------------------\n");	
#endif	
/*
		posX=posxy->x;
		posY=posxy->y;		
*/		
		//posX=posxy.x;
		//posY=posxy.y;
		
		if(count<5){			
			posX=testPOSx[count];
			posY=testPOSy[count];
			count++;
		}else{
			count=0;			
		}
#if 1		
		printf("posX:%f\nposY:%f\n",posX,posY);	
		printf("------------------\n");	
#endif		
		//usleep(100000);	
		//usleep(500000);	
		sleep(2);	
		
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
#if choice
	//賴捲	
	rssiDataB(rk,&count,rssi);

#else
	//俊賢	
	rssiDataA(rk,&count,rssi,&start);
#endif
		fclose(in);	
		usleep(800000);	
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
#if choice	
	//賴捲	
	rssiDataB(rk,&count,rssi);
#else
	//俊賢
	rssiDataA(rk,&count,rssi,&start);	
#endif
		fclose(in);	
		usleep(800000);
		
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
#if choice
	//賴捲	
	rssiDataB(rk,&count,rssi);
#else
	//俊賢	
	rssiDataA(rk,&count,rssi,&start);	
#endif		
		fclose(in);	
		usleep(800000);			
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
#if choice	
	//賴捲	
	rssiDataB(rk,&count,rssi);
#else
	//俊賢	
	rssiDataA(rk,&count,rssi,&start);	
#endif		
		fclose(in);	
		usleep(800000);	
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
		pauseOLED=0;
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
		pauseOLED=0;
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
	oledInfo.posX=0;
	oledInfo.posY=0;
	dealOLEDData(&oledInfo);
	while(1){
		//oledInfo.posX=計算後的數值;
		//oledInfo.posY=計算後的數值;
		//oledInfo.number=DBnumber;	
		oledInfo.posX=posX*100;		
		oledInfo.posY=posY*100;
		if(pauseOLED==1){
			dealOLEDData(&oledInfo);
			usleep(100000);	
		}
		
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
		//publishDBData(1,100,200,1022);		//for test
		publishDBData(DBnumber,posX,posY,lightSensor);	
		//publishData("type your data");		//fortest can publsih data to mqtt server
		sleep(1);
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
			pauseOLED=1;
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
void RSSI_BUBSort(double *sortData,int rk){
	int i,j;
	double temp;
	int sc=RTQty;	
	int minor=rk+1;
#if 1
	printf("minor:%d排序前:",minor);
	for(i=0;i<RTQty;i++){
		printf("%f ",sortData[i]);
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
#if 1
	printf("minor:%d排序後:",minor);	
	for(i=0;i<RTQty;i++){
		printf("%f ",sortData[i]);
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
//俊賢RSSI取後處理
//-------------------------
void rssiDataA(int rk,int *count,double *rssi,int *start){
	int mid;
	double avg=0;
	int cc=*count;
	//RSSI[rk]=RSSItemp[rk];	
	printf("RSSItemp[%d]:%d\n",rk,RSSItemp[rk]);
	if(abs(RSSItemp[rk])>0 && RSSItemp[rk]>-72){
		rssi[cc]=RSSItemp[rk];	
		cc++;
		*count=cc;
		//printf("count:%d\n",*count);
	}	
	if(cc==RTQty){		
		cc=0;
		*count=cc;
		//avg = avgRSSI(rssi , RTQty); //RSSI取平均
		//RSSI[rk]=avg;		
		RSSI_BUBSort(rssi,rk);	//排序	
		int num=3;
		int i=0;
		//排序後的最小最大值不列入平均
		for(i=2;i<=num;i++){
			avg=avg+rssi[i];
		}
		avg=avg/(num-1);		
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
	printf("RSSI[%d]:%f\n",rk,RSSI[rk]);		
	TXP[rk]= TXPtemp[rk];
}
//-------------------------

//-------------------------
//賴捲RSSI取後處理
//-------------------------
void rssiDataB(int rk,int *count,double *rssi){
    double avg;
	int cc=*count;
	//printf("RSSItemp[%d]:%d\n", rk,RSSItemp[rk] );     
    if( RSSItemp[rk] != 0 && RSSItemp[rk]>-72)
    {
        rssi[cc]=RSSItemp[rk];
		//printf("%f %f %d\n", cc, rssi[cc],RSSItemp[rk] );  
			//printf(" rssi[%d]:%f RSSItemp[%d]:%d\n",cc, rssi[cc],rk,RSSItemp[rk] );      		
        cc++;
        *count = cc % RTQty;
        
        avg = avgRSSI(rssi , RTQty); //RSSI取平均
        //printf("avgRSSI: %f\n", avg);
        RSSI[rk] = avg;
		//printf("RSSI[%d]:%f\n",rk, avg);
        TXP[rk]= TXPtemp[rk];
		
    }
}
//-------------------------

//-----------------------------------------------------------------------
