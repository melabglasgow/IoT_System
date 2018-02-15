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
#ifndef __PRED_H__
#define __PRED_H__

#include "mbed.h"

#define CoAP_PRED_OBJ	"16"
#define CoAP_PRED_INFO	"0"
#define CoAP_PRED_CUR	"1"
#define CoAP_PRED_DES	"2"

#define shift(bit) (1 << bit)
#define NO_PRED 4.0

enum comf_feel{
	hot			= shift(0),
	okay		= shift(1),
	cold		= shift(2),
};


class PRED
{
public:

	PRED(void) : comf(NO_PRED){
				
		//**************************** CoAP resource init ****************************//
		
		pred_object = M2MInterfaceFactory::create_object(CoAP_PRED_OBJ);
		M2MObjectInstance* pred_inst = pred_object->create_object_instance();
		
		string info = "some info on the resource";
		M2MResource* res_info = pred_inst->create_static_resource(CoAP_PRED_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());
		
		
		M2MResource* res_cur = pred_inst->create_dynamic_resource(CoAP_PRED_CUR, "cur",
		M2MResourceInstance::STRING, true /* observable */);
		res_cur->set_operation(M2MBase::GET_ALLOWED);
		res_cur->set_value((uint8_t*)"0",1);
		
		M2MResource* res_des = pred_inst->create_dynamic_resource(CoAP_PRED_DES, "comf-ash",
		M2MResourceInstance::STRING, true /* observable */);
		res_des->set_operation(M2MBase::PUT_ALLOWED);
		res_des->set_value((uint8_t*)"0",1);
		//value_updated_callback value_updated_function;
		
		res_des->set_value_updated_function(value_updated_callback(this,&PRED::des_updated));
		
		//****************************************************************************//
	}
	
	void des_updated(const char* x) {
		printf("\n[PRED::des_updated]: ");
		M2MObjectInstance* inst = pred_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PRED_DES);
		
		String payload_M2M = res->get_value_string();
		string payload(payload_M2M.c_str());		// Convert from M2M String to std::string
		printf("string: %s\n",payload.c_str());
		comf = ::atof(payload.c_str());
		
		printf("float: %f\n",comf);
		
		if(comf>1.5)
			comf_level = hot;
		else if(comf<-1.5)
			comf_level = cold;
		else
			comf_level = okay;
		
	};

	void mdc_set_cur(float temp,float humi){
		printf("[PRED::mdc_set_cur]: ");
		M2MObjectInstance* inst = pred_object->object_instance();
		M2MResource* res = inst->resource(CoAP_PRED_CUR);
		
		char buffer[300];
		
		//int size = sprintf (buffer,"MLModelId=\'ml-nk5kcVLRaMv\',Record={\'Internal Temperature\': \'%f\',\'Internal humidity\': \'%f\'},PredictEndpoint=\'https://realtime.machinelearning.eu-west-1.amazonaws.com\'",temp,humi);
		//int size = sprintf (buffer,"Record={\'Internal Temperature\': \'%f\',\'Internal humidity\': \'%f\'},PredictEndpoint=\'https://realtime.machinelearning.eu-west-1.amazonaws.com\'",temp,humi);
		int size = sprintf (buffer,"{\"temperature\": \"%f\",\"humidity\": \"%f\"}",temp,humi);
		res->set_value((uint8_t*)buffer, size);
		
		printf("\tprediction input: %.*s\n", size, buffer);	
	}
	
	M2MObject* get_object() {
        return pred_object;
    }
public:
	double comf;
	comf_feel comf_level;
private:
	M2MObject*  pred_object;
};

#endif //__PRED_H__