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
#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "mbed.h"

#define CoAP_BUTTON_OBJ		"12"
#define CoAP_BUTTON_INFO	"0"
#define CoAP_BUTTON_COUNT	"1"


class BUTTON : public InterruptIn{
public:
    
	BUTTON(PinName pin) : InterruptIn(pin) , count(0) , clicked(false) {       							// create the InterruptIn on the pin specified to Counter
		this->fall(this, &BUTTON::callback); 								// attach increment function of this counter instance
		printf("\r\n********* Button Count Initialised *********\r\n");
		
		button_object = M2MInterfaceFactory::create_object(CoAP_BUTTON_OBJ);
		M2MObjectInstance* button_inst = button_object->create_object_instance();
		
		string info = "receive the number times the button has been pressed";
		M2MResource* res_info = button_inst->create_static_resource(CoAP_BUTTON_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());

		M2MResource* res_count = button_inst->create_dynamic_resource(CoAP_BUTTON_COUNT, "count",
			M2MResourceInstance::STRING, true /* observable */);
		res_count->set_operation(M2MBase::GET_ALLOWED);
		res_count->set_value((uint8_t*)"0", 1);
    }
	
	void mdc_update_count(){
		printf("[BUTTON::mdc_update_count]: ");
		
		M2MObjectInstance* inst = button_object->object_instance();
		M2MResource* res = inst->resource(CoAP_BUTTON_COUNT);
		
		// serialize the value of counter as a string, and tell connector
		char buffer[3];
		int size = sprintf (buffer, "%d", count);
		res->set_value((uint8_t*)buffer, size);
		printf("count updated to %i \n",count);
	}
	
    void callback() {
		printf("[BUTTON::callback]: clicked = true, count++, osSignalSet(mainThread, 0x1)\n");
		clicked = true;
        count++;
		//mdc_update_count();
		osSignalSet(mainThread, 0x1); //updates_global.release();
    }
		
	M2MObject* get_object() {
        return button_object;
    }
	
public:
	bool clicked;
	uint8_t count;
private:
	M2MObject*  button_object;
};

#endif //__BUTTON_H__