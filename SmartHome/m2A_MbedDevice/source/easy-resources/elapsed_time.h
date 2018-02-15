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

#ifndef __ELAPSED_TIME_H__
#define __ELAPSED_TIME_H__

#include "mbed.h"

#define CoAP_ETIME_OBJ		"15"
#define CoAP_ETIME_INFO		"0"
#define CoAP_ETIME_VALUE	"1"

class ELAPSED_T : public Timer
{
public:

	ELAPSED_T() : Timer(){
		this->start();
		printf("\r\n********* timer started *********\r\n");
		
		etime_object = M2MInterfaceFactory::create_object(CoAP_ETIME_OBJ);
		
		M2MObjectInstance* etime_inst = etime_object->create_object_instance();
		etime_inst->set_operation(M2MBase::DELETE_ALLOWED);
		
		string info = "you can GET the time (format mm:ss) and request for an update of the timer value by POST \"UPDATE\" or reset by POST \"RESET\"";
		M2MResource* res_info = etime_inst->create_static_resource(CoAP_ETIME_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());
			
		M2MResource* res_value = etime_inst->create_dynamic_resource(CoAP_ETIME_VALUE, "time",
			M2MResourceInstance::STRING, true /* observable */);
		res_value->set_operation(M2MBase::GET_POST_ALLOWED);
		res_value->set_value((uint8_t*)"0", 1);
		res_value->set_delayed_response(true);
		res_value->set_execute_function(execute_callback(this, &ELAPSED_T::post_handle));
	}
	
	void mdc_get_value(void){
		M2MObjectInstance* inst = etime_object->object_instance();
		M2MResource* res = inst->resource(CoAP_ETIME_VALUE);
		
		printf("time value GET from server (%s): ",res->get_value_string().c_str());
	}
	
	void mdc_update_value(bool update_by_post = false){
		printf("[ELAPSED_T::mdc_update_value]: ");
		
		M2MObjectInstance* inst = etime_object->object_instance();
		M2MResource* res = inst->resource(CoAP_ETIME_VALUE);
		
		_time = this->read();
		uint8_t minutes = _time/60;
		uint8_t seconds = _time%60;
		
		char buffer[5];
		int size = sprintf (buffer, "%d:%d",minutes,seconds);
		
		res->set_value((uint8_t*)buffer, size);
		if(update_by_post) {
			if(res->send_delayed_post_response())
				printf("post delayed response success");
			else
				printf("post delayed response error");
		}
		
			
		printf("time updated: %.*s\n", size, buffer);
	}
	
	void post_handle(void *argument) {
		printf("[ELAPSED_T::post_handle]: ");
		// check if POST contains payload
		if (argument) {
			M2MResource::M2MExecuteParameter* param = (M2MResource::M2MExecuteParameter*)argument;
			int payload_length = param->get_argument_value_length();
			const uint8_t* payload = param->get_argument_value();
			
			char buffer[payload_length];
			sprintf(buffer,"%.*s",payload_length, payload);
			
			string str(buffer);
			printf("command received (%s): ",str.c_str());
			
			if(str.compare("UPDATE") == 0) {
				printf("valid update command\n");
				mdc_update_value(true);
			} else if(str.compare("RESET") == 0){
				printf("Timer Reset\n");
				this->reset();
			} else {
				printf("invalid update command\n");
			}	
		}
	}

	M2MObject* get_object() {
        return etime_object;
    }

private:
	M2MObject*  etime_object;
	uint16_t _time;

};

#endif // __ELAPSED_TIME_H__