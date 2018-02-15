/*
 * Copyright (c) 2015, 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* SpwfInterface NetworkSocketAPI Example Program
 * Copyright (c) 2015 ARM Limited
 * Copyright (c) 2017 KLIKA TECH, LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 
/* Credits
* This application uses Open Source components.
* Modified by Sergio Martin (Github: @smysergio)
* Inspired on Project: https://os.mbed.com/teams/Klika-Tech/code/Nucleo-AWS-IoT-mbed/ https://github.com/Klika-Tech/nucleo-aws-iot-demo
* License: Apache-2.0
*/

#ifndef __PING_H__
#define __PING_H__

#include "mbed.h"
#include "SpwfInterface.h"
#include "TCPSocket.h"
#include "MQTTClient.h"
#include "MQTTWiFi.h"

#include <string> 

#define MQTT_MAX_PACKET_SIZE 350
#define MQTT_MAX_PAYLOAD_SIZE 300
#define MAX_BW	300

#define TOPIC_SUB_START	"Nucleo/direct/start"
#define TOPIC_SUB_RECV	"Nucleo/direct/ping-in"
#define TOPIC_PUB_SEND	"Nucleo/direct/ping-out"
#define TOPIC_PUB_TIME	"Nucleo/direct/time"
#define TOPIC_SHADOW	"$aws/things/Nucleo/shadow/update"

int subscribe_recv(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack);
int ping_send(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack);
void ping_recv(MQTT::MessageData & msgMQTT);
void ping_start_handle(MQTT::MessageData & msgMQTT);

string time_batch;
bool ping_enable = false;
bool ping_flag = true;
uint16_t ping_time;
uint16_t count = 0;
Timer ping_timer;

char *buf_mbw;
bool max_bw = false;

void ping_init(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack){
	printf("[PING::ping_init]: ");
	int result = client->subscribe(TOPIC_SUB_RECV, MQTT::QOS0, ping_recv);
	if (!result)	printf ("subscribed (%s) ",TOPIC_SUB_RECV);
	result = client->subscribe(TOPIC_SUB_START, MQTT::QOS0, ping_start_handle);
	if (!result)	printf ("subscribed (%s)\n",TOPIC_SUB_START);
	ping_timer.start();
	
	buf_mbw = (char*) calloc(MAX_BW,sizeof(char));
	for(int i = 0; i<(MAX_BW-1);i++){
		//itoa(i%10,buf_mbw[i],1);
		sprintf(buf_mbw+i,"%i",i%10);
		//buf_mbw[i] = '1';
	}
	*(buf_mbw+MAX_BW) = '/0';
	printf("\n\r%s",buf_mbw);
}

int ping_batch_time(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack){
	printf("[PING::batch_time]: ");
	
	MQTT::Message message;
	char* pubTopic = TOPIC_PUB_TIME;

	
	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;
	message.payload = (void*)time_batch.c_str();
	message.payloadlen = time_batch.length();
	
	printf("Length (%d)\nMessage %s\n\r", time_batch.length(), time_batch.c_str());

	return client->publish(pubTopic, message);	
}

void ping_store_time(){
	printf("[PING::ping_store_time]: ");
	
	char buffer[5];
	sprintf(buffer,"%i",ping_time);
	string time_string(buffer);
	time_batch = time_batch + time_string + ",";
	printf("time_stored, time_batch size (%i)\n",time_batch.length());
	//printf("time_batch (size %i) :(%s) ",time_batch.length(),time_batch.c_str());
	ping_flag = true;
}

int ping_send(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack){
	printf("[PING::ping_send]: ");
	
	char* pubTopic = TOPIC_PUB_SEND;
	int flag = 0;
	count++;

	if(max_bw){
		
		MQTT::Message *message;
		message = new MQTT::Message;

		message->qos = MQTT::QOS0;
		message->retained = false;
		message->dup = false;
		
		printf("(max BW) ");
		
		message->payload = (void*)buf_mbw;
		message->payloadlen = strlen(buf_mbw);
		printf("ping send (%i)\tMessage is very long!\n\r",count);
		
		flag = client->publish(pubTopic, *message);
		delete message;
		
	}else {
		
		MQTT::Message message;
		
		message.qos = MQTT::QOS0;
		message.retained = false;
		message.dup = false;
	
		printf("(min BW) ");
		
		char buffer[10];
		sprintf (buffer, "%d", count);
		
		message.payload = (void*)buffer;
		message.payloadlen = strlen(buffer);
		printf("ping send (%i)\tMessage (%s), length (%i)\n\r",count,buffer,strlen(buffer));
		
		flag = client->publish(pubTopic, message);
	}
	
	ping_timer.reset();
	
	ping_flag = false;
	
	return flag;
}

/*int ping_send(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack){
	printf("[PING::ping_send]: ");
	
	char* pubTopic = TOPIC_PUB_SEND;
	int flag = 0;
	count++;
	
	MQTT::Message message;
		
	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;
		
	if(max_bw){
		
		printf("(max BW) ");
		
		message.payload = (void*)buf_mbw;
		message.payloadlen = strlen(buf_mbw);
		printf("ping send (%i)\tMessage is very long!\n\r",count);
		
		flag = client->publish(pubTopic, message);
		
	}else {
		
		printf("(min BW) ");
		
		char buffer[10];
		sprintf (buffer, "%d", count);
		
		message.payload = (void*)buffer;
		message.payloadlen = strlen(buffer);
		printf("ping send (%i)\tMessage (%s), length (%i)\n\r",count,buffer,strlen(buffer));
		
		flag = client->publish(pubTopic, message);
	}
	
	ping_timer.reset();
	
	ping_flag = false;
	
	return flag;
}*/

/*int ping_send(MQTT::Client<MQTTWiFi, Countdown, MQTT_MAX_PACKET_SIZE>* client, MQTTWiFi* ipstack){
	printf("[PING::ping_send]: ");
	
	char* pubTopic = TOPIC_PUB_SEND;
	int flag = 0;
	count++;
	
	MQTT::Message *message;
	message = new MQTT::Message;

	message->qos = MQTT::QOS0;
	message->retained = false;
	message->dup = false;
	
	if(max_bw){

		printf("(max BW) ");
		
		message->payload = (void*)buf_mbw;
		message->payloadlen = strlen(buf_mbw);
		printf("ping send (%i)\tMessage is very long!\n\r",count);
		
	}else {
		
		printf("(min BW) ");
		
		char buffer[10];
		sprintf (buffer, "%d", count);
		
		message->payload = (void*)buffer;
		message->payloadlen = strlen(buffer);
		printf("ping send (%i)\tMessage (%s), length (%i)\n\r",count,buffer,strlen(buffer));
	}
	
	flag = client->publish(pubTopic, *message);
	delete message;
	ping_timer.reset();
	
	ping_flag = false;
	
	return flag;
}*/

void ping_recv(MQTT::MessageData & msgMQTT) {
	printf("\n[PING::recv]: ");
	printf("msgMQTT size (%i) ",msgMQTT.message.payloadlen);
	
	if(max_bw){
		printf("(max BW) ");
		sprintf(buf_mbw,"%.*s",msgMQTT.message.payloadlen,(uint8_t*)msgMQTT.message.payload);
		printf("Message is very long!\n\r");
		
	}else {
		printf("(min BW) ");
		char buffer[msgMQTT.message.payloadlen];
		sprintf(buffer,"%.*s",msgMQTT.message.payloadlen,(uint8_t*)msgMQTT.message.payload);
		string str(buffer);
		printf("ping received (%s) ",str.c_str());
	}
	
	ping_time = ping_timer.read_ms();
	printf("\tping time (%i)\n\r", ping_time);
	
	ping_store_time(); //ping_flag = true;
	
}

void ping_start_handle(MQTT::MessageData & msgMQTT) {
	printf("[PING::ping_start_handle]: ");
	printf("msgMQTT size (%i) ",msgMQTT.message.payloadlen);
	
	char buffer[msgMQTT.message.payloadlen];
	sprintf(buffer,"%.*s",msgMQTT.message.payloadlen,(uint8_t*)msgMQTT.message.payload);
	string cmd_string(buffer);
	printf("command received (%s): ",cmd_string.c_str());	
	
	if(cmd_string.compare("START") == 0) {
		ping_enable = true;
		printf("valid START command\n\r");
	} else if(cmd_string.compare("STOP") == 0) {
		ping_enable = false;
		count = 0;
		printf("valid STOP command\n\r");
	} else {
		printf("invalid update command\n\r");
	}	
}

#endif //__PING_H__