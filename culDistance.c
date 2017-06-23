//-----------------------------------------------------------------------
//程式版本:v1.0
//修改日期:2017/06/17
//是否驗證:yes
//計算距離函式
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

