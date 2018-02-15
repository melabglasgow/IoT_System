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
  
#ifndef __SENSE_H__
#define __SENSE_H__

#include "mbed.h"
#include "x_nucleo_iks01a1.h"

#define shift(bit) (1 << bit)

#define X	0
#define Y	1
#define Z	2

#define CoAP_SENSE_OBJ	"14"

#define CoAP_SENSE_INFO	"0"
#define CoAP_SENSE_GYRO	"1"
#define CoAP_SENSE_ACCE	"2"
#define CoAP_SENSE_MAGN	"3"
#define CoAP_SENSE_HUMI	"4"
#define CoAP_SENSE_PRES	"5"
#define CoAP_SENSE_TEMP "6"
#define CoAP_SENSE_MOVE "7"

extern osThreadId mainThread;

enum sensor{
	GYRO	= shift(0),
	ACCE	= shift(1),
	MAGN	= shift(2),
	HUMI	= shift(3),
	PRES	= shift(4),
	TEMP	= shift(5),
	ALL		= 63,
};

class SENSE
{
public:
	SENSE(PinName i2c_1, PinName i2c_2)
	{	
		gyro = (int32_t*) malloc(3);
		acce = (int32_t*) malloc(3);
		magn = (int32_t*) malloc(3);
		
		humi = 0; pres = 0; temp = 0;
		
		_update = ALL;
		
		motion = false;
		
		sense_expansion_board = X_NUCLEO_IKS01A1::Instance(i2c_1, i2c_2);
		gyroscope = sense_expansion_board->GetGyroscope();
		accelerometer = sense_expansion_board->GetAccelerometer();
		magnetometer = sense_expansion_board->magnetometer;
		humidity_sensor = sense_expansion_board->ht_sensor;
		pressure_sensor = sense_expansion_board->pt_sensor;
		temp_sensor = sense_expansion_board->ht_sensor;
		
		id();
		update_sensors();
		print();
		thread = NULL;
		
		//**************************** CoAP resource init ****************************//
		
		_mdc_update = ALL;
		
		sense_object = M2MInterfaceFactory::create_object(CoAP_SENSE_OBJ);
		M2MObjectInstance* sense_inst = sense_object->create_object_instance();
		
		string info = "GET the resources as strings";
		M2MResource* res_info = sense_inst->create_static_resource(CoAP_SENSE_INFO, "info",
			M2MResourceInstance::STRING,(uint8_t*)info.c_str(), info.length());
		
		M2MResource* res_gyro = sense_inst->create_dynamic_resource(CoAP_SENSE_GYRO, "gyro",
		M2MResourceInstance::STRING, true /* observable */);
		res_gyro->set_operation(M2MBase::GET_ALLOWED);
		res_gyro->set_value((uint8_t*)"0",1);
		
		M2MResource* res_acce = sense_inst->create_dynamic_resource(CoAP_SENSE_ACCE, "acce",
		M2MResourceInstance::STRING, true /* observable */);
		res_acce->set_operation(M2MBase::GET_ALLOWED);
		res_acce->set_value((uint8_t*)"0",1);
		
		M2MResource* res_magn = sense_inst->create_dynamic_resource(CoAP_SENSE_MAGN, "magn",
		M2MResourceInstance::STRING, true /* observable */);
		res_magn->set_operation(M2MBase::GET_ALLOWED);
		res_magn->set_value((uint8_t*)"0",1);
		
		M2MResource* res_humi = sense_inst->create_dynamic_resource(CoAP_SENSE_HUMI, "humi",
		M2MResourceInstance::STRING, true /* observable */);
		res_humi->set_operation(M2MBase::GET_ALLOWED);
		res_humi->set_value((uint8_t*)"0",1);
		
		M2MResource* res_pres = sense_inst->create_dynamic_resource(CoAP_SENSE_PRES, "pres",
		M2MResourceInstance::STRING, true /* observable */);
		res_pres->set_operation(M2MBase::GET_ALLOWED);
		res_pres->set_value((uint8_t*)"0",1);
		
		M2MResource* res_temp = sense_inst->create_dynamic_resource(CoAP_SENSE_TEMP, "temp",
		M2MResourceInstance::STRING, true /* observable */);
		res_temp->set_operation(M2MBase::GET_ALLOWED);
		res_temp->set_value((uint8_t*)"0",1);
		
		M2MResource* res_move = sense_inst->create_dynamic_resource(CoAP_SENSE_MOVE, "motion",
		M2MResourceInstance::STRING, true /* observable */);
		res_move->set_operation(M2MBase::GET_ALLOWED);
		res_move->set_value((uint8_t*)"0",1);

		//****************************************************************************//
	}
	
	void mdc_set_sensors(int cmd = -1){
		printf("[SENSE::mdc_set_sensors]:\n");
		
		if(cmd != -1)
			_mdc_update = cmd;
		
		M2MObjectInstance* inst = sense_object->object_instance();
		
		char buffer[40];
		
		if( (_mdc_update & TEMP) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_TEMP);	
			int size = sprintf (buffer, "%2.2f",temp);
			res->set_value((uint8_t*)buffer, size);
			printf("\tTEMP updated in mdc: %.*s\n", size, buffer);
		}
		if( (_mdc_update & HUMI) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_HUMI);
			int size = sprintf (buffer, "%2.2f",humi);
			res->set_value((uint8_t*)buffer, size);
			printf("\tHUMI updated in mdc: %.*s\n", size, buffer);
		}
		if( (_mdc_update & PRES) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_PRES);
			int size = sprintf (buffer, "%2.2f",pres);
			res->set_value((uint8_t*)buffer, size);
			printf("\tPRES updated in mdc: %.*s\n", size, buffer);
		}
		if( (_mdc_update & MAGN) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_MAGN);
			int size = sprintf (buffer, "x:%i, y:%i, z:%i",magn[0],magn[1],magn[2]);
			res->set_value((uint8_t*)buffer, size);
			printf("\tMAGN updated in mdc: %.*s\n", size, buffer);
		}
		if( (_mdc_update & ACCE) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_ACCE);
			int size = sprintf (buffer, "x:%i, y:%i, z:%i",acce[0],acce[1],acce[2]);
			res->set_value((uint8_t*)buffer, size);
			printf("\tACCE updated in mdc: %.*s\n", size, buffer);
		}
		if( (_mdc_update & GYRO) != 0){
			M2MResource* res = inst->resource(CoAP_SENSE_GYRO);
			int size = sprintf (buffer, "x:%i, y:%i, z:%i",gyro[0],gyro[1],gyro[2]);
			res->set_value((uint8_t*)buffer, size);
			printf("\tGYRO updated in mdc: %.*s\n", size, buffer);
		}
	}
	
	void update_sensors(int cmd = -1, bool print = true) {	// print flag to avoid excessive printing in motion thread
		if(print)
			printf("\n[SENSE::update_sensors]: ");
		
		if(cmd != -1)
			_update = cmd;
		if(print)
			printf("Update ID: %i ",_update);
		
		if( (_update & TEMP) != 0)
			temp_sensor->get_temperature(&temp);
		if( (_update & HUMI) != 0)
			humidity_sensor->get_humidity(&humi);
		if( (_update & PRES) != 0)
			pressure_sensor->get_pressure(&pres);
		if( (_update & MAGN) != 0)
			magnetometer->get_m_axes(magn);
		if( (_update & ACCE) != 0)
			accelerometer->get_x_axes(acce);
		if( (_update & GYRO) != 0)
			gyroscope->get_g_axes(gyro);
	}
	
	void id() {
		uint8_t id;
		printf("\r\n********* sense test *********\r\n");
		humidity_sensor->read_id(&id);
		printf("HTS221  humidity & temperature    = 0x%X\r\n", id);
		pressure_sensor->read_id(&id);
		printf("LPS25H  pressure & temperature    = 0x%X\r\n", id);
		magnetometer->read_id(&id);
		printf("LIS3MDL magnetometer              = 0x%X\r\n", id);
		gyroscope->read_id(&id);
		printf("LSM6DS0 accelerometer & gyroscope = 0x%X\r\n", id);
	}
	
	void print() {
		printf("\n[SENSE::print]: ");
		printf("\r\n");
		printf("-------------------------------------------------\r\n");
		printf("HTS221: [temp] %2.2f C",temp);
		printf("LPS25H: [hum]  %2.2f %%,    [press] %2.2f mbar\r\n", humi,pres);
		printf("---\r\n");
		printf("LSM6DS0 [gyro/mdps]:   %7ld, %7ld, %7ld\r\n", gyro[0], gyro[1], gyro[2]);
		printf("LSM6DS0 [acc/mg]:      %7ld, %7ld, %7ld\r\n", acce[0], acce[1], acce[2]);
		printf("LIS3MDL [mag/mgauss]:  %7ld, %7ld, %7ld\r\n", magn[0], magn[1], magn[2]);		
		printf("-------------------------------------------------\r\n");
	}
	
	// we have to wait until the main loop starts in the endpoint before we create our thread... 
	void thread_enable() {
		printf("[SENSE::thread_enable]: ");
		if(thread == NULL) {
			thread = new Thread;
			printf("sense thread Created! ");
		}
		if(thread != NULL) {
			thread->start(callback(this,&SENSE::sense_thread));
			printf("sense thread started!");
	    }else {
			printf("thread is NULL!");
		}
		printf("\n");
	}
	
	void update_sensors_onThread(int cmd) {
		_update = cmd;
        _event.set(0x1);
    }
	
	void mdc_set_motion(void){
		printf("[SENSE::mdc_set_motion]:\n");
	
		
		M2MObjectInstance* inst = sense_object->object_instance();
		M2MResource* res = inst->resource(CoAP_SENSE_MOVE);
		
		char buffer[20];
		int size;
		if(motion)
			size = sprintf (buffer,"true");
		else
			size = sprintf (buffer,"false");
		res->set_value((uint8_t*)buffer, size);
		printf("\tmotion set to: %.*s\n", size, buffer);
		motion = false;
	}
	
	void sense_thread(void) {
		
		int32_t prev[3];			// previous accelerations
		int32_t diff[3] = {0,0,0};	// difference in accelerations
		
		// Calibration Loop
		for(int i = 0; i < 100; i++) {
			_event.wait_all(0x1,100);
			update_sensors(ACCE,false);
			for(int i = 0; i < 3; i++){
				prev[i] = acce[i];
			}
		}
		
		while (true) {
            _event.wait_all(0x1,100);
			update_sensors(ACCE,false);
			
			for(int i = 0; i < 3; i++){
				diff[i] = acce[i] - prev[i];
				prev[i] = acce[i];
				
				//printf("\n\r--<{%i:%d}>--\n", i, diff[i]);
				
				if ( (diff[i]>500) && (diff[i]<700) && !motion ){ // limit to 700 to remove random >1000 error
					printf("[SENSE::sense_thread]: ");
					printf("(%i:%d) ", i, diff[i]);
					printf("motion detected!\n");
					motion = true;
				}
			}
			Thread::yield();
        }
    }
	
	M2MObject* get_object() {
        return sense_object;
    }
	
private:
	X_NUCLEO_IKS01A1 *sense_expansion_board;
	Thread  *thread;
	int _update;
	int _mdc_update;
	EventFlags _event;
	M2MObject*  sense_object;
	

public:
	GyroSensor		*gyroscope;
	MotionSensor	*accelerometer;
	MagneticSensor	*magnetometer;
	HumiditySensor	*humidity_sensor;
	PressureSensor	*pressure_sensor;
	TempSensor		*temp_sensor;

public:
	int32_t *gyro;
	int32_t *acce;
	int32_t *magn;
	float humi, pres, temp, temp2;
	bool motion;
};

#endif //__SENSE_H__