//-----------------------------------------------------------------------
//程式版本:v1.1
//修改日期:2017/06/22
//是否驗證:yes
//計算距離函式
//新增Kalman Filter
//-----------------------------------------------------------------------
#include "culDistance.h"

//-------------------------
//dealDistance
//-------------------------
double dealDistance(int txPower,double rssi,int pot){
	
	double ratio = rssi*1.0/txPower;
	double distance;
	
	if (ratio < 1.0) {		
		distance=pow(ratio, 10);	    
	}
	else{	  
		distance =  (0.89976)*pow(ratio, 7.7095) + 0.111;   	   
	}	
#if 0
	printf("ratio:%f\n",ratio);
	printf("distance%d:%f\n",pot,distance);
#endif

	return distance;
}
//-------------------------

//-------------------------
//dealDistance
//-------------------------
double dealDistance_DISoffSet(int txPower,double rssi,int pot,float  DISoffest){

	double ratio = rssi*1.0/txPower;
	double distance;	
	if (ratio < 1.0) {		
		distance=pow(ratio, 10);	    
	}
	else{	  
		distance =  (0.89976)*pow(ratio, 7.7095) + 0.111;   	   
	}	
	distance=distance+ DISoffest;
#if 0
	printf("ratio:%f\n",ratio);
	printf("distance%d:%f\n",pot,distance);
#endif	
	return distance;
}
//-------------------------

//-------------------------
//Kalman filter
//-------------------------
double kalmanFilter(double inData)
{
    float prevData=0;
    float p=10, q=0.008, r=1, kGain=0;
    
    //Kalman filter function start*******************************
    p = p+q;
    kGain = p/(p+r);
    
    inData = prevData+(kGain*(inData-prevData));
    
    p = (1-kGain)*p;
    
    prevData = inData;
    //Kalman filter function stop********************************
    
    return inData;
}
//-------------------------

