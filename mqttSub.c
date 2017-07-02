//-----------------------------------------------------------------------
/*
 * test-sub.c
 * Part of the mosquito-test demonstration application
 * Consumes messages from an MQTT topic indefinitely
 * Copyright (c)2016 Kevin Boone. Distributed under the terms of the 
 *  GPL v3.0 
 */
//MQTT publish函式庫
//程式版本:v1.0
//修改日期:2017/06/19
//是否驗證:yes
//注意事項
//topic需寫固定 不然會跟publish受影響
//-----------------------------------------------------------------------
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include <time.h>
#include "mqttSub.h"
// Server connection parameters
//#define MQTT_HOSTNAME "192.168.6.36" 
#define MQTT_PORT 1883
#define MQTT_USERNAME "admin1"
#define MQTT_PASSWORD "admin1"
#define MQTT_TOPIC "control"
char hostName[12];
struct mosquitto *mosqSub = NULL;
int ret;
char subData[6];
//-------------------------
//對此函式庫做初始化
//-------------------------
void mqttsubInit(char *hostd){  
	cleanBUF(subData);
	sprintf(hostName,"%s",hostd);
#if 1	
	printf("hostName:%s\n",hostName);
#endif		

  mosquitto_lib_init();  //Initialize the Mosquitto library
  
  //Create a new Mosquito runtime instance with a random client ID,
  //and no application-specific callback data.  
  mosqSub = mosquitto_new (NULL, true, NULL);
  if (!mosqSub){
    fprintf (stderr, "Can't init Mosquitto library\n");
    exit (-1);
  }

  //Set up username and password
  mosquitto_username_pw_set (mosqSub, MQTT_USERNAME, MQTT_PASSWORD);

  //Establish a connection to the MQTT server. Do not use a keep-alive ping 
  ret = mosquitto_connect (mosqSub, hostName, MQTT_PORT, 0);
  if (ret) {   
    fprintf (stderr, "Can't connect to Mosquitto server888\n");
    exit (-1);
   }	
}	
//-------------------------	

//-------------------------	
//my_message_callback. 
//Called whenever a new message arrives 
//------------------------- 
void my_message_callback(struct mosquitto *mosq, void *obj, 
    const struct mosquitto_message *message)
  {
  // Note: nothing in the Mosquitto docs or examples suggests that we
  //must free this message structure after processing it.
#if 1	
	printf ("Got message: %s\n", (char *)message->payload);
	sprintf(subData,"%s",(char *)message->payload);
	//printf ("subData:%s\n",subData);
	/*
	printf ("subData[0]:%c ",subData[0]);
	printf ("subData[1]:%c ",subData[1]);
	printf ("subData[2]:%c ",subData[2]); 
	printf ("subData[3]:%c ",subData[3]); 
	printf ("subData[4]:%c\n",subData[4]);		
	*/
#endif	
}
//-------------------------

//-------------------------
//subscribehtmlData
//-------------------------
void subscribehtmlData(){
	ret = mosquitto_subscribe(mosqSub, NULL,MQTT_TOPIC, 0);
	if (ret){
		fprintf (stderr, "Can't publish to Mosquitto server\n");
		exit (-1);
    }
    //char getdata[40];
	//Specify the function to call when a new message is received
	cleanBUF(subData);	
	mosquitto_message_callback_set (mosqSub, my_message_callback);
	
		//Wait for new messages
	mosquitto_loop_forever (mosqSub, -1, 1);		
}	
//-------------------------

//-------------------------
//cleanBUF
//-------------------------
void cleanBUF(char *buf){
	int size=sizeof(buf);
	int i;
	for(i=0;i<=size;i++){
		buf[i]='0';
	}
}
//-------------------------

//-------------------------
//MQTT close
//-------------------------
void mqttsubClose(){
 // Tody up. In this simple example, this point is never reached. We can
  //force the mosquitto_loop_forever call to exit by disconnecting
  //the session in the message-handling callback if required.
  mosquitto_destroy (mosqSub);
  mosquitto_lib_cleanup();
}
//-------------------------  
  
/* 原始程式碼
int main (int argc, char **argv)
  {
  //struct mosquitto *mosq = NULL;
 
  // Initialize the Mosquitto library
  mosquitto_lib_init();

  // Create a new Mosquito runtime instance with a random client ID,
  //  and no application-specific callback data.  
  mosq = mosquitto_new (NULL, true, NULL);
  if (!mosq)
    {
    fprintf (stderr, "Can't init Mosquitto library\n");
    exit (-1);
    }

  // Set up username and password
  mosquitto_username_pw_set (mosq, MQTT_USERNAME, MQTT_PASSWORD);

  // Establish a connection to the MQTT server. Do not use a keep-alive ping
  int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
  if (ret)
    {
    fprintf (stderr, "Can't connect to Mosquitto server\n");
    exit (-1);
    }

  // Subscribe to the specified topic. Multiple topics can be
  //  subscribed, but only one is used in this simple example.
  //  Note that we don't specify what to do with the received
  //  messages at this point
  ret = mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);
  if (ret)
    {
    fprintf (stderr, "Can't publish to Mosquitto server\n");
    exit (-1);
    }
 
  // Specify the function to call when a new message is received
  mosquitto_message_callback_set (mosq, my_message_callback);

  // Wait for new messages
  mosquitto_loop_forever (mosq, -1, 1);

  // Tody up. In this simple example, this point is never reached. We can
  //  force the mosquitto_loop_forever call to exit by disconnecting
  //  the session in the message-handling callback if required.
  mosquitto_destroy (mosq);
  mosquitto_lib_cleanup();
  
  return 0;
  }  
 */

