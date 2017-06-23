//-----------------------------------------------------------------------
/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Consumes messages from an MQTT topic indefinitely
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the 
 *  GPL v3.0 
 */
//MQTT subscribe函式庫
//程式版本:v1.0
//修改日期:2017/06/19
//是否驗證:yes
//注意事項
//-----------------------------------------------------------------------
#ifndef MQTTSUB_H
#define MQTTSUB_H
void mqttsubInit(char *hostd);
void mqttsubClose();
void subscribehtmlData();
void cleanBUF(char *buf);
#endif