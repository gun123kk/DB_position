//-----------------------------------------------------------------------
/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Publishes a fixed number of simple messages to a topic 
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the 
 *  GPL v3.0 
 */
//MQTT publish�禡�w
//�{������:v1.0
//�ק���:2017/06/19
//�O�_����:yes
//�`�N�ƶ�
//-----------------------------------------------------------------------
#ifndef MQTTPUB_H
#define MQTTPUB_H
void mqttpubInit(char *hostd);
void publishDBData(int num,double posX,double posY,int lightVal);
void publishData(char *data);
void publishMessage(char *data);
void mqttpubClose();

#endif
