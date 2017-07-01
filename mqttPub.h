//-----------------------------------------------------------------------
/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Publishes a fixed number of simple messages to a topic 
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the 
 *  GPL v3.0 
 */
//MQTT publish函式庫
//程式版本:v1.1
//修改日期:2017/06/27
//是否驗證:yes
//注意事項
//v1.1修改void publishDBData(int num,double posX,double posY,int lightVal)
//修改為	void publishDBData(int num,int area,int lightVal)
//-----------------------------------------------------------------------
#ifndef MQTTPUB_H
#define MQTTPUB_H
void mqttpubInit(char *hostd);
void publishDBData(int num,int area,int lightVal);
void publishData(char *data);
void publishMessage(char *data);
void mqttpubClose();

#endif
