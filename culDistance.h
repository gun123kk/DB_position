//-----------------------------------------------------------------------
//程式版本:v1.1
//修改日期:2017/06/22
//是否驗證:yes
//計算距離函式
//新增Kalman Filter
//-----------------------------------------------------------------------

#ifndef CULDISTANCE_H
#define CULDISTANCE_H
#include <math.h>
double dealDistance(int txPower,double rssi,int pot);
double dealDistance_DISoffSet(int txPower,double rssi,int pot,float DISoffest);
double kalmanFilter(double inData);
#endif


