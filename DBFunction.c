//-----------------------------------------------------------------------
//程式版本:v1.5
//修改日期:2017/06/27
//是否驗證:yes
//注意事項
//v1.1使用函式庫<bcm2835.h>
//v1.1新增函式: 
//1.DC MOTOR ON/OFF控制 void DCMotorONOFF(int st);
//2.按鈕控制馬達 void BtnColDCM();
//3.信號控制DC MOTOR 帶延遲時間 DCMotorONOFFwithDT(dcmotor *st);
//4.按鈕控制蜂鳴器與馬達 void BtnColLED_Buzzer();
//5.信號控制蜂鳴器與馬達 void LED_BUZ_ONOFFwithCT(ledBuz* st);
//6.OLED輸出
//v1.2更新:將距離函式移出double dealDistance(int txPower,double rssi,int pot);
//v1.3新增函式庫mcp3008.h for lightSensor
//v1.4OLED新增靠近哪顆IBEACON會顯示near number
//v1.5修改OLED顯示 NUM AREA nearibeacon
//-----------------------------------------------------------------------
#include "DBFunction.h"
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <my_debug.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <bcm2835.h>
#include "I2C_SSD1306Z.h"
#include <mcp3008.h>


//#define LED1 5
#define COM 22
#define BTN_DCM 24
#define BTN_LED_BUZ 23
#define CDCM	18
#define Buzzer 16
#define SPI_CS 0		//light sensor

int CLED[5]={5,6,12,13,26};
int ledNum=sizeof(CLED)/sizeof(int);
//-------------------------
//OLED
int lineEnd;
char buffer[17]={0};
//-------------------------


//-------------------------
//對此函式庫做初始化
//-------------------------
void DBF_init(){
	if ( !bcm2835_init() ) {
       perror("Error");
       //return 1;
   }	 
	i2c_start();   
	bcm2835_gpio_fsel(COM, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(COM, LOW);		//關閉gpio 22(shield board總開關)
	//count=0;
	DCMotorInit();			//初始化DCMOTOR
	BtnInit();				//初始化按鈕	
	LEDInit();				//初始化LED
	BuzzerInit();			//初始化蜂鳴器
	//printf("ledNum:%d\n",ledNum);
	OLEDInit();				//初始化OLED
	bcm2835_spi_begin();	//初始化SPI	
}
//-------------------------

//-------------------------
//BTN INIT
//DC MOTOR使用按鈕(GPIO24) 控制
//蜂鳴器和LED使用按鈕(GPIO23) 控制
//-------------------------
void BtnInit(){
	bcm2835_gpio_fsel(BTN_DCM, BCM2835_GPIO_FSEL_INPT);
	bcm2835_gpio_fsel(BTN_LED_BUZ, BCM2835_GPIO_FSEL_INPT);	
}
//-------------------------

//-------------------------
//DC MOTOR INIT
//使用GPIO18
//-------------------------
void DCMotorInit(){
	bcm2835_gpio_fsel(CDCM, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_write(CDCM,LOW);
	//bcm2835_gpio_fsel(LED1, BCM2835_GPIO_FSEL_OUTP);
	//bcm2835_gpio_write(LED1, LOW);
}
//-------------------------

//-------------------------
//lED INIT
//使用GPIO5 6 13 26 12
//-------------------------
void LEDInit(){	
	int i=0;
	for( i=0;i<ledNum;i++){
		bcm2835_gpio_fsel(CLED[i], BCM2835_GPIO_FSEL_OUTP);	
		bcm2835_gpio_write(CLED[i],HIGH);
	}	
}
//-------------------------

//-------------------------
//Buzzer INIT
//使用GPIO5 16
//-------------------------
void BuzzerInit(){
	bcm2835_gpio_fsel(Buzzer, BCM2835_GPIO_FSEL_OUTP);	
	bcm2835_gpio_write(Buzzer,LOW);	
}
//-------------------------

//-------------------------
//OLED INIT
//-------------------------
void OLEDInit(){
	Init_LCD();
	clear_LCD();
}
//-------------------------

//-------------------------
//DC MOTOR ON/OFF控制
//0:off 1:on
//-------------------------
void DCMotorONOFF(int st){
	if(st)		
		bcm2835_gpio_write(CDCM, HIGH);
	else		
		bcm2835_gpio_write(CDCM, LOW);
}
//-------------------------

//-------------------------
//信號控制 DC MOTOR ON/OFF控制
//-------------------------
void DCMotorONOFFwithDT(dcmotor *st){
	if(st->onOff){
		DCMotorONOFF(1);		
		int sp=st->delayTime;
		sleep(sp);
		//bcm2835_delay(1000);		//ms
		DCMotorONOFF(0);	
		sleep(1);
	}	
}
//-------------------------


//-------------------------
//按鈕控制DC MOTOR
//使用GPIO24
//-------------------------
void BtnColDCM(){	
	int st= bcm2835_gpio_lev(BTN_DCM);		
	//normal=high
	if(st){
		DCMotorONOFF(0);
		//printf("high\n");
	}else{				
		usleep(20000);		
		DCMotorONOFF(1);
		//count++;	
		while(!st){			
			usleep(10000);
			st= bcm2835_gpio_lev(BTN_DCM);	
		}
		//printf("count:%d\n",count);
		//printf("low\n");
	}
}
//-------------------------

//-------------------------
//按鈕控制外接LED和蜂鳴器
//-------------------------
void BtnColLED_Buzzer(){
	int st= bcm2835_gpio_lev(BTN_LED_BUZ);		
	//normal=high
	if(st){
		BuzzerONOFF(0);
		LEDONOFF(0);
		//printf("high\n");
	}else{				
		BuzzerONOFF(1);
		LEDONOFF(1);		
		//count++;	
		while(!st){			
			usleep(10000);
			st= bcm2835_gpio_lev(BTN_LED_BUZ);	
		}
		//printf("count:%d\n",count);
		//printf("low\n");
	}	
}
//-------------------------

//-------------------------
//LED ON/OFF控制
//0:off 1:on
//-------------------------
void LEDONOFF(int st){
	int i;
	if(st){	
		for(i=0;i<ledNum;i++){
			bcm2835_gpio_write(CLED[i],LOW);	
		}
	}else{
		for(i=0;i<ledNum;i++){
			bcm2835_gpio_write(CLED[i],HIGH);	
		}		
	}
}
//-------------------------

//-------------------------
//Buzzer ON/OFF控制
//0:off 1:on
//-------------------------
void BuzzerONOFF(int st){
	if(st){	
		bcm2835_gpio_write(Buzzer,HIGH);	
	}else{		
		bcm2835_gpio_write(Buzzer,LOW);				
	}
}
//-------------------------

//-------------------------
//信號控制 Buzzer LED ON/OFF控制
//-------------------------
void LED_BUZ_ONOFFwithCT(ledBuz *st){
	int i;
	if(st->onOff){
		for(i=0;i<st->count;i++){
			BuzzerONOFF(1);
			LEDONOFF(1);		
			bcm2835_delay(200);		
			BuzzerONOFF(0);
			LEDONOFF(0);	
			bcm2835_delay(200);	
		}
	}	
}
//-------------------------

//-------------------------
//對此函式庫 GPIO做關閉
//-------------------------
void DBF_close(){
	//bcm2835_gpio_write(LED1,LOW);
	bcm2835_gpio_write(COM,LOW);	
	bcm2835_gpio_write(CDCM,LOW);
	bcm2835_gpio_write(BTN_LED_BUZ,LOW);
	bcm2835_gpio_write(Buzzer,LOW);
	int i;
	for( i=0;i<ledNum;i++){		
		bcm2835_gpio_write(CLED[i],HIGH);
	}	
	bcm2835_spi_end();
}
//-------------------------

//-------------------------
//OLED 資料處理
//-------------------------
void dealOLEDData(oled *info){
	int i;
	for(i=0;i<sizeof(buffer)/sizeof(char);i++){
		buffer[i]=' ';
	}	
	/*
	OLED_display(buffer,lineEnd,0);	
	OLED_display(buffer,lineEnd,1);
	OLED_display(buffer,lineEnd,2);	
	usleep(1000);	
	*/
	lineEnd=sprintf(buffer," NUM:%d",info->number);
	OLED_display(buffer,lineEnd,0);	
	lineEnd=sprintf(buffer," area:%d",info->area);
	OLED_display(buffer,lineEnd,1);		
	if(info->near==0){
		lineEnd=sprintf(buffer," near ibeacon:%c%c",'N','o');
	}else{
		lineEnd=sprintf(buffer," near ibeacon:%d",info->near);
	}
	OLED_display(buffer,lineEnd,2);	
}
//-------------------------

//-------------------------
//OLED display
//-------------------------
void OLED_display(char *data,int length,int number){
	data[length]=' ';
	print_Line(number,data);	
#if 0
	printf("%s\n",data);
#endif
}
//-------------------------

//-------------------------
//light sensor
//-------------------------
int getLightSensorVal(){
	int val;
	val=MCP3008_analog_read( SPI_CS , 0);
#if 0
	printf("lightSensr:%d\n",val);
#endif
	usleep(10000);
	return val;	
}
//-------------------------











