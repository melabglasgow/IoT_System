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
#ifndef __M_LED_H__
#define __M_LED_H__

#include "mbed.h"

#define CoAP_LED_OBJ	"11"
#define CoAP_LED_INFO	"0"
#define CoAP_LED_STATUS	"1"

#define OFF			0
#define ON 			1
#define TOGGLE		2
#define INVALID 	-1

class LED: private DigitalOut
{
public:

	LED(PinName ledPin) : DigitalOut(ledPin) , status(false) {
		led_test(20);
		
		led_object = M2MInterfaceFactory::create_object(CoAP_LED_OBJ);
		M2MObjectInstance* led_inst = led_object->create_object_instance();
		
		string info = "PUT state of the led as a string \"ON\" or \"OFF\"";
		M2MResource* res_info = led_inst->create_static_resource(CoAP_LED_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());
			
		M2MResource* res_status = led_inst->create_dynamic_resource(CoAP_LED_STATUS, "status",
				M2MResourceInstance::STRING, true /* observable */);
		res_status->set_operation(M2MBase::PUT_ALLOWED);
		res_status->set_value((uint8_t*)"OFF", 3);
		res_status->set_value_updated_function(value_updated_callback(this,&LED::status_updated));
	}
	
	void status_updated(const char* x) {
		printf("\n[LED::status_updated]: ");
		M2MObjectInstance* inst = led_object->object_instance();
		M2MResource* res = inst->resource(CoAP_LED_STATUS);
		
		String payload_M2M = res->get_value_string();
		string payload(payload_M2M.c_str());		// Convert from M2M String to std::string
		int8_t status_update = string_to_status(payload); 
		
		printf("\n\n\n\n DEBUG: (%s) (%i)", payload.c_str(), payload.length());
		if(status_update != INVALID) {
			status = status_update;
			printf("status updated to: %s (%i)\n",payload.c_str(),status);
			if(status != TOGGLE)
				this->write(!(this->read()));
			else
				this->write(status);
		} else {
			printf("invalid led status received: %s\n",payload.c_str());
		}
		
	};
	
	int8_t string_to_status(string led_status) {
		if(led_status.compare("\"ON\"") == 0)
			return ON;
		else if(led_status.compare("\"OFF\"") == 0)
			return OFF;
		else if(led_status.compare("\"TOGGLE\"") == 0)
			return TOGGLE;
		else
			return INVALID;
		
	}
	
	void led_test(char cycles){
		printf("\r\n********* led test *********\r\n");
		for(int i = 0; i < cycles; i++) {
			this->write(!(this->read()));
			wait(0.1);
		}
	}
	
	M2MObject* get_object() {
        return led_object;
    }
	
public:
	uint8_t status;
	M2MObject*  led_object;
};

#endif //__M_LED_H__