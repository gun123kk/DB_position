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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <mosquitto.h>
#include "mqttPub.h"

// Server connection parameters
//#define MQTT_HOSTNAME "192.168.6.22" 
#define MQTT_PORT 1883
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_TOPIC "test87"
char hostName[12];
//char topic[100];
struct mosquitto *mosqPub = NULL;
int ret;
//-------------------------
//對此函式庫做初始化
//-------------------------
void mqttpubInit(char *hostd){
	
	sprintf(hostName,"%s",hostd);
	//sprintf(topic,"%s",topicd);
#if 0	
	printf("hostName:%s\n",hostName);
	//printf("topic:%s\n",topic);
#endif			
	mosquitto_lib_init();		  // Initialize the Mosquitto library
	
	//Create a new Mosquito runtime instance with a random client ID,
	//and no application-specific callback data.  
	mosqPub = mosquitto_new (NULL, true, NULL);	
	if (!mosqPub){
		fprintf (stderr, "Can't initialize Mosquitto library\n");
		exit (-1);
    }

	mosquitto_username_pw_set (mosqPub, MQTT_USERNAME, MQTT_PASSWORD);
 
	//Establish a connection to the MQTT server. Do not use a keep-alive ping
	ret = mosquitto_connect (mosqPub, hostName, MQTT_PORT, 0);
	if (ret){
		fprintf (stderr, "Can't connect to Mosquitto server\n");
		exit (-1);
    }		
}
//-------------------------


//-------------------------
//MQTT　data
//-------------------------
void publishData(char *data){
	char *send = malloc(sizeof(data));  
	sprintf(send ,"%s",data);
#if 0
	printf("publsih data:%s\n",send );
#endif	
	publishMessage(send);
	free(send);
}
//-------------------------



//-------------------------
//MQTT　DB data
//-------------------------
void publishDBData(int num,int area,int lightVal){	
	char text[100];	
    //sprintf (text, "{\"num\":%d,\"posX\":%.2f,\"posY\":%.2f,\"light\":%d}",num,posX,posY,lightVal);
	sprintf (text, "{\"num\":%d,\"area\":%d,\"light\":%d}",num,area,lightVal);
	publishMessage(text);	
}
//-------------------------

//-------------------------
//MQTT　publish
//-------------------------
void publishMessage(char *data){
	//Publish the message to the topic
	ret = mosquitto_publish (mosqPub, NULL, MQTT_TOPIC, 
      strlen (data), data, 0, false);
    if (ret){      
      fprintf (stderr, "Can't publish to Mosquitto server\n");
      exit (-1);
    }
	//We need a short delay here, to prevent the Mosquito library being
	//torn down by the operating system before all the network operations
	//are finished.
	//usleep(1000);	
	//sleep(1);	
}
//-------------------------

//-------------------------
//MQTT close
//-------------------------
void mqttpubClose(){
	// Tidy up
	mosquitto_disconnect (mosqPub);
	mosquitto_destroy (mosqPub);
	mosquitto_lib_cleanup();	
}
//-------------------------



/* 原始程式碼
int main (int argc, char **argv)
  {
  struct mosquitto *mosq = NULL;
 
  // Initialize the Mosquitto library
  mosquitto_lib_init();

  // Create a new Mosquito runtime instance with a random client ID,
  //  and no application-specific callback data.  
  mosq = mosquitto_new (NULL, true, NULL);
  if (!mosq)
    {
    fprintf (stderr, "Can't initialize Mosquitto library\n");
    exit (-1);
    }

  mosquitto_username_pw_set (mosq, MQTT_USERNAME, MQTT_PASSWORD);

  // Establish a connection to the MQTT server. Do not use a keep-alive ping
  int ret = mosquitto_connect (mosq, MQTT_HOSTNAME, MQTT_PORT, 0);
  if (ret)
    {
    fprintf (stderr, "Can't connect to Mosquitto server\n");
    exit (-1);
    }

  int i;
  char text[40];
  for (i = 0; i < 10; i++)
    {

    sprintf (text, "{\"num:\"%d,\"posx:\"%d,\"posy:\"%d,\"light:\"%d}",1,87,99,512);
    // Publish the message to the topic
    ret = mosquitto_publish (mosq, NULL, MQTT_TOPIC, 
      strlen (text), text, 0, false);
    if (ret)
      {
      fprintf (stderr, "Can't publish to Mosquitto server\n");
      exit (-1);
      }
    }

  // We need a short delay here, to prevent the Mosquito library being
  //  torn down by the operating system before all the network operations
  //  are finished.
  sleep (1);

  // Tidy up
  mosquitto_disconnect (mosq);
  mosquitto_destroy (mosq);
  mosquitto_lib_cleanup();
  
  return 0;
  }
*/
