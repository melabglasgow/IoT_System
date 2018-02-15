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
 
#ifndef __RGB_H__
#define __RGB_H__

#include "mbed.h"

#define shift(bit) (1 << bit)
#define rgbAll rgb1 = rgb2 = rgb3

#define CoAP_RGB_OBJ	"13"
#define CoAP_RGB_INFO	"0"
#define CoAP_RGB_COLOR	"1"

enum color{
	no_color	= -1,
	red			= shift(0),
	green		= shift(1),
	blue		= shift(2),
	white		= 7,
	off			= 0
};

static const enum color color_map[5] = { red, green, blue, white, off};

class RGB
{
public:

	RGB(PinName redpin, PinName bluepin, PinName greenpin){
		rgb = new BusOut(redpin,bluepin,greenpin);
		rgb_test(2);
		
		color_counter = 0;			
		//**************************** CoAP resource init ****************************//
		
		rgb_object = M2MInterfaceFactory::create_object(CoAP_RGB_OBJ);
		M2MObjectInstance* rgb_inst = rgb_object->create_object_instance();
		
		string info = "options: RED, BLUE, GREEN, WHITE, OFF";
		M2MResource* res_info = rgb_inst->create_static_resource(CoAP_RGB_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());
		
		M2MResource* res_color = rgb_inst->create_dynamic_resource(CoAP_RGB_COLOR, "color",
		M2MResourceInstance::STRING, true /* observable */);
		res_color->set_operation(M2MBase::GET_PUT_ALLOWED);
		res_color->set_value((uint8_t*)"white",10);
		//value_updated_callback value_updated_function;
		
		res_color->set_value_updated_function(value_updated_callback(this,&RGB::color_updated));
		
		//****************************************************************************//
	}
	
	void color_updated(const char* x) {
		printf("\n[RGB::color_updated]: ");
		M2MObjectInstance* inst = rgb_object->object_instance();
		M2MResource* res = inst->resource(CoAP_RGB_COLOR);
		
		String payload_M2M = res->get_value_string();
		string payload(payload_M2M.c_str());		// Convert from M2M String to std::string
		
		color color_update = string_to_color(payload); 
		
		if(color_update != no_color) {
			printf("color updated to: %s (%i)\n",payload.c_str(),color_update);
			write(color_update);
		} else {
			printf("invalid rgb color received: %s\n",payload.c_str());
		}
		
	}

	void mdc_set_color(color led_color){
		printf("[RGB::mdc_set_color]: ");
		M2MObjectInstance* inst = rgb_object->object_instance();
		
		M2MResource* res = inst->resource(CoAP_RGB_COLOR);
				
		// serialize the value of counter as a string, and tell connector
		
		string str = color_to_string(led_color);
		char *cstr = new char[str.length()];
		strcpy(cstr, str.c_str());
		res->set_value((uint8_t*)cstr, str.length());
		printf("color set to %s \n",str.c_str());
		delete [] cstr;
	}
	
	
	void flash(void) {
		printf("[RGB::write]: ");
		write(color_map[color_counter]);
		color_counter++;
		if(color_counter > 3)	color_counter = 0;
	}
	
	void write(color led_color) {  //bool mdc_update
		printf("[RGB::write]: ");
		status = led_color;
		*rgb = led_color;
		printf("color ID %i \n",led_color);
		if (true)									//if (mdc_update)
			mdc_set_color(led_color);
	}
	
	void rgb_test(char cycles){
		printf("\r\n********* rgb test *********\r\n");
		int rgb_counter = 0;
		int i = 0;
		while(i < cycles) {
			
			*rgb = color_map[rgb_counter];
			
			if(rgb_counter == 4) {
				rgb_counter = 0;
				i++;
			}else {
				rgb_counter++;
			}
			wait(0.2);
		}
		
	}
	
	string color_to_string(color led_color) {
		if(led_color == red)
			return "red";
		else if(led_color == green)
			return "green";
		else if(led_color == blue)
			return "blue";
		else if(led_color == white)
			return "white";
		else if(led_color == off)
			return "off";
		else
			return NULL;
	}
	
	color string_to_color(string led_color) {
		if(led_color.compare("\"RED\"") == 0)
			return red;
		else if(led_color.compare("\"BLUE\"") == 0)
			return blue;
		else if(led_color.compare("\"GREEN\"") == 0)
			return green;
		else if(led_color.compare("\"WHITE\"") == 0)
			return white;
		else if(led_color.compare("\"OFF\"") == 0)
			return off;
		else
			return no_color;	
	}
	
	M2MObject* get_object() {
        return rgb_object;
    }
	
public:
	color status;
private:
	uint8_t color_counter;				   
	BusOut *rgb;
	M2MObject*  rgb_object;
};

#endif //__RGB_H__