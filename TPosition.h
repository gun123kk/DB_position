#ifndef TPosition_h
#define TPosition_h


struct posXY{
	double x;
	double y;
};
typedef struct posXY POS; 
//POS* TPosition(double dis1, double dis2, double dis3, double dis4,float areaX,float areaY);
void TPosition(double dis1, double dis2, double dis3, double dis4,float areaX,float areaY,POS *pxy);
//void TPosition(double dis1, double dis2, double dis3, double dis4);
#endif /* TPosition_h */
