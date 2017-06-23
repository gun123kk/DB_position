//-----------------------------------------------------------------------
//程式版本:v1.0
//修改日期:2017/06/17
//是否驗證:yes
//計算距離函式
//-----------------------------------------------------------------------

#ifndef CULDISTANCE_H
#define CULDISTANCE_H
#include <math.h>
double dealDistance(int txPower,double rssi,int pot);
double dealDistance_DISoffSet(int txPower,double rssi,int pot,float DISoffest);
#endif


