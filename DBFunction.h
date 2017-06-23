//-----------------------------------------------------------------------
//程式版本:v1.3
//修改日期:2017/06/19
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
//-----------------------------------------------------------------------

#ifndef DBFUNCTION_H
#define DBFUNCTION_H
struct DCM{
	int delayTime;
	int onOff;
};
typedef struct DCM dcmotor; 

struct LEDBUZ{
	int count;
	int onOff;
};
typedef struct LEDBUZ ledBuz;

struct OLED_info{
	int number;
	int posX;
	int posY;
};
typedef struct OLED_info oled; 

//int count; 測試彈跳使用
void DBF_init(void);
void BtnInit(void);
void LEDInit(void);				
void BuzzerInit(void);			
void OLEDInit(void);
void DCMotorONOFF(int st);
void DCMotorONOFFwithDT(dcmotor *st);
void DCMotorInit(void);
void BtnColDCM(void);
void BtnColLED_Buzzer(void);
void LEDONOFF(int st);
void BuzzerONOFF(int st);
void LED_BUZ_ONOFFwithCT(ledBuz* st);
void OLED_display(char *data,int length,int number);
void dealOLEDData(oled *info);
int getLightSensorVal();
#endif


