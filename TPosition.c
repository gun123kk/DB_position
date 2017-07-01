//
//  TPosition.c
//  
//  AX=Y -> X=(((A^T)A)^-1)*(A^T)*Y
//  B=A^T , C=BA , D=C^-1 , E=DB , X=EY
//
//

#include "TPosition.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>


//POS* TPosition(double dis1, double dis2, double dis3, double dis4,float areaX,float areaY)
void TPosition(double dis1, double dis2, double dis3, double dis4,float area[][2],POS *pxy)
//void TPosition(double dis1, double dis2, double dis3, double dis4)
{
//    float dis1 = sqrt(pow(17, 2)+pow(21, 2));
//    float dis2 = sqrt(pow(17, 2)+pow(1, 2));
//    float dis3 = sqrt(pow(13, 2)+pow(21, 2));
//    float dis4 = sqrt(pow(13, 2)+pow(1, 2));
    
    double P[4][2] = { {area[0][0],area[0][1]} , {area[1][0],area[1][1]} , {area[2][0],area[2][1]} , {area[3][0],area[3][1]} };
//    double A[3][2] = { {0,areaY} , {areaX,0} , {areaX,areaY} };
    double A[3][2];
    double pos[2];		//0 x,1 y
    double Y[3];
    double B[2][3];
    double C[2][4]={0};
    double D[2][2];
    double E[2][3];
    //-----Y矩陣-----
    Y[0]=pow(dis1,2)-pow(dis2, 2)-pow(P[0][0], 2)+pow(P[1][0], 2)-pow(P[0][1], 2)+pow(P[1][1], 2);
    Y[1]=pow(dis1,2)-pow(dis3, 2)-pow(P[0][0], 2)+pow(P[2][0], 2)-pow(P[0][1], 2)+pow(P[2][1], 2);
    Y[2]=pow(dis1,2)-pow(dis4, 2)-pow(P[0][0], 2)+pow(P[3][0], 2)-pow(P[0][1], 2)+pow(P[3][1], 2);
    //--------------
    
    //-----P矩陣運算成Ａ-----
    A[0][0]=P[1][0]-P[0][0];
    A[1][0]=P[2][0]-P[0][0];
    A[2][0]=P[3][0]-P[0][0];
    A[0][1]=P[1][1]-P[0][1];
    A[1][1]=P[2][1]-P[0][1];
    A[2][1]=P[3][1]-P[0][1];
    //-----------------
    
    //-----A矩陣轉置成B-----
    int i,j,k;
    for (i=0; i<=2 ;i++)
    {
        for (j=0; j<=1; j++)
        {
            B[j][i]=A[i][j];
        }
    }
    //-----------------
    
    //-----BA相乘成C-----
    for(i=0; i<=1; i++)
    {
        for(j=0; j<=1; j++)
        {
            C[i][j]=0;
            for (k=0; k<=2; k++)
            {
                C[i][j]=C[i][j]+(2*B[i][k])*(2*A[k][j]);
            }
            //printf("C[%d][%d]=%f\n", i,j,C[i][j]);
        }
    }
    //---------------
    
    //-----C矩陣的反矩陣D-----
    double m;
    for(i=0; i<2; i++)
        C[i][2+i]=1;
    
    for(i=0; i<2; i++)
    {
        for(j=i+1; j<2; j++)
        {
            m=C[j][i]/C[i][i];
            for(k=i; k<4; k++)
                C[j][k]-=m*C[i][k];
        }
    }
    
    for(i=1; i>0; i--)
    {
        for(j=i-1; j>=0; j--)
        {
            m=C[j][i]/C[i][i];
            for(k=i; k<4; k++)
                C[j][k]-=m*C[i][k];
        }
    }
    
    for(i=0; i<2; i++)
    {
        m=C[i][i];
        for(j=0; j<4; j++)
            C[i][j]/=m;
    }
    
    for(i=0; i<2; i++)
    {
        for(j=2; j<4; j++)
            D[i][j-2]=C[i][j];
    }
    
    //----------------------
    
    //-----D矩陣乘B矩陣成E-----
    for(i=0; i<=1; i++)
    {
        for(j=0; j<=2; j++)
        {
            E[i][j]=0;
            for (k=0; k<=1; k++)
            {
                E[i][j]=E[i][j]+D[i][k]*(2*B[k][j]);
            }
            //printf("E[%d][%d]=%f\n", i,j,E[i][j]);
        }
    }
    //-----------------------
    
    //-----E矩陣乘Y矩陣成X-----
    for(i=0; i<=1; i++)
    {
        pos[i]=0;
        for (k=0; k<=2; k++)
        {
            pos[i]=pos[i]+E[i][k]*Y[k];
        }
    }
    //-----------------------
    
    //    int a, b;
    //    for(a = 0; a<=1 ; a++)
    //    {
    //        for(b = 0; b<=1 ; b++)
    //        {
    //            printf("D[%d][%d]=%f\n", a,b,D[a][b]);
    //        }
    //    }
    //    for(a = 0; a<=1 ; a++)
    //    {
    //        for(b = 0; b<=2 ; b++)
    //        {
    //            printf("B[%d][%d]=%f\n", a,b,B[a][b]);
    //        }
    //    }
	
	pxy->x= pos[0];
	pxy->y= pos[1];		
	/*
	POS *pp = malloc(sizeof(POS));  
    //printf("X:%f\nY:%f\n", pos[0], pos[1]);
    pp->x= pos[0];
	pp->y= pos[1];	
	//printf("done\n");
	return pp;
	*/
    //return 0;
}


