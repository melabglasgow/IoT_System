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
 
 /* Credits
  * This application uses Open Source components.
  * Created by Sergio Martin (Github: @smysergio)
  * Inspired on Project: https://github.com/ARMmbed/mbed-os-example-client
  * License: Apache-2.0
  */
 
  
#ifndef __PING_H__
#define __PING_H__

#include "mbed.h"

#define MAX_BW	1000

#define CoAP_PING_OBJ	"1"
#define CoAP_PING_SEND	"1"
#define CoAP_PING_RECV	"2"
#define CoAP_PING_TIME	"3"

extern osThreadId mainThread;

class PING
{
public:

	PING(void) :
	
		count(0), ping_flag(true), ping_enable(false), max_bw(false) 
	
	{
		printf("\r[PING:constructor]: ");	
		//**************************** CoAP resource init ****************************//
		
		ping_object = M2MInterfaceFactory::create_object(CoAP_PING_OBJ);
		M2MObjectInstance* ping_inst = ping_object->create_object_instance();

		M2MResource* res_send = ping_inst->create_dynamic_resource(CoAP_PING_SEND, "send",
		M2MResourceInstance::STRING, true /* observable */);
		res_send->set_operation(M2MBase::GET_ALLOWED);
		res_send->set_value((uint8_t*)"0",1);

		M2MResource* res_recv = ping_inst->create_dynamic_resource(CoAP_PING_RECV, "receive",
		M2MResourceInstance::STRING, true /* observable */);
		res_recv->set_operation(M2MBase::PUT_ALLOWED);
		res_recv->set_value((uint8_t*)"0",1);
		//value_updated_callback value_updated_function;

		res_recv->set_value_updated_function(value_updated_callback(this,&PING::recv));
		
		M2MResource* res_time = ping_inst->create_dynamic_resource(CoAP_PING_TIME, "time",
		M2MResourceInstance::STRING, true /* observable */);
		res_time->set_operation(M2MBase::GET_POST_ALLOWED);
		res_time->set_value((uint8_t*)"0",1);
		res_time->set_execute_function(execute_callback(this, &PING::ping_start_handle));
		
		//****************************************************************************//
		ping_timer.start();
		for(int i = 0; i < MAX_BW; i++) {
			buf_mbw[i] = 3;
		}
	}
	
	/*void mdc_time(){
		printf("[PING::mdc_time]: ");
		M2MObjectInstance* inst = ping_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PING_TIME);
		// serialize the value of counter as a string, and tell connector
		char buffer[10];
		int size = sprintf (buffer, "%d,%d",count,ping_time);
		res->set_value((uint8_t*)buffer, size);
		ping_flag = true;
		osSignalSet(mainThread, 0x1);
		printf("\t time send\n");	
	}*/
	
	void batch_time(){
		printf("[PING::batch_time]: ");
		M2MObjectInstance* inst = ping_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PING_TIME);
		// serialize the value of counter as a string, and tell connector
		printf("time_batch (%s): ",time_batch.c_str());
		res->set_value((uint8_t*)time_batch.c_str(),time_batch.length());
		ping_flag = true;
		osSignalSet(mainThread, 0x1);
		printf("\t time send\n");	
	}
	
	void store_time(){
		printf("[PING::store_time]: ");
		char buffer[5];
		sprintf(buffer,"%i",ping_time);
		string str(buffer);
		time_batch = time_batch + str + ",";
		printf(" time_batch size (%i)\n",time_batch.length());
		//printf("time_batch (size %i) :(%s) ",time_batch.length(),time_batch.c_str());
		ping_flag = true;
		osSignalSet(mainThread, 0x1);
	}
	
	void send(){
		printf("[PING::send]: ");
		M2MObjectInstance* inst = ping_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PING_SEND);
		// serialize the value of counter as a string, and tell connector
		count++;
		
		if(max_bw){
			buf_mbw[0] = count;
			res->set_value((uint8_t*)buf_mbw, MAX_BW);
			printf("(max BW) ");
		}else {
			char buffer[10];
			int size = sprintf (buffer, "%d", count);
			res->set_value((uint8_t*)buffer, size);
			printf("(min BW) ");
		}
		
		
		printf("count (%i) ",count);
		ping_timer.reset();
		ping_flag = false;
		printf("\tping send\n");
		
	}
	
	void recv(const char* x) {
		printf("\n[PING::recv]: ");
		
		// Read timer
		ping_time = ping_timer.read_ms();
		
		// Read value
		M2MObjectInstance* inst = ping_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PING_RECV);
		
		String payload_M2M = res->get_value_string();
		printf("size (%i) ",payload_M2M.length());
		
		if(max_bw){
			printf("(max BW) ");
			string too_large = "too large";
			printf("string: %s\t",too_large.c_str());
		}else {
			printf("(min BW) ");
			string payload(payload_M2M.c_str());		// Convert from M2M String to std::string
			printf("string: (%s)\t",payload_M2M.c_str());
		}
		
		printf("ping time (%i)\n",ping_time);
		store_time();		
	}
	
	void ping_start_handle(void *argument) {
		printf("[PING::post_handle]: ");
		// check if POST contains payload
		if (argument) {
			M2MResource::M2MExecuteParameter* param = (M2MResource::M2MExecuteParameter*)argument;
			int payload_length = param->get_argument_value_length();
			uint8_t* payload = param->get_argument_value();
			
			char buffer[payload_length];
			sprintf(buffer,"%.*s",payload_length, payload);
			
			string str(buffer);
			printf("command received (%s): ",str.c_str());
			
			if(str.compare("START") == 0) {
				ping_enable = true;
				printf("valid START command\n");
				osSignalSet(mainThread, 0x1);
			} else if(str.compare("STOP") == 0) {
				ping_enable = false;
				count = 0;
				printf("valid STOP command\n");
			} else {
				printf("invalid update command\n");
			}	
		}
	}
	
	M2MObject* get_object() {
        return ping_object;
    }
	
public:
	bool ping_enable;
	bool ping_flag;
	uint16_t count;
	bool max_bw;
	
private:
	Timer ping_timer;
	string time_batch;
	uint16_t ping_time;
	uint8_t buf_mbw[MAX_BW];
	
	M2MObject*  ping_object;
};

#endif //__PING_H__