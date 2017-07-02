//-----------------------------------------------------------------------
/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Consumes messages from an MQTT topic indefinitely
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the 
 *  GPL v3.0 
 */
//MQTT subscribe�禡�w
//�{������:v1.0
//�ק���:2017/06/19
//�O�_����:yes
//�`�N�ƶ�
//-----------------------------------------------------------------------
#ifndef MQTTSUB_H
#define MQTTSUB_H
void mqttsubInit(char *hostd);
void mqttsubClose();
void subscribehtmlData();
void cleanBUF(char *buf);
#endif