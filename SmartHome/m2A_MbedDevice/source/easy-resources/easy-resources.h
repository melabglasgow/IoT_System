#ifndef __EASY_RESOURCES_H__
#define __EASY_RESOURCES_H__

#include "mbed.h"

#include "rgb.h"
#include "sense.h"
#include "button_count.h"
#include "elapsed_time.h"
#include "m_led.h"
#include "pred.h"

class RES
{
public:
	
	RES(M2MObjectList *object_list){
		printf("\n----------- Initialising Easy-Resources -----------\r\n");
		#if SENSE_RESOURCE == true
			sense = new SENSE(SENSE_PINS);
			object_list->push_back(sense->get_object());
		#endif

		#if RGB_RESOURCE == true
			rgb = new RGB(RGB_PINS); //RGB pins
			object_list->push_back(rgb->get_object());
		#endif

		#if BUTTON_RESOURCE == true
			button = new BUTTON(BUTTON_PIN);
			object_list->push_back(button->get_object());
		#endif
		
		#if LED_RESOURCE == true
			led = new LED(LED_PIN);
			object_list->push_back(led->get_object());
		#endif
		
		#if TIMER_RESOURCE == true
			e_time = new ELAPSED_T;
			object_list->push_back(e_time->get_object());
		#endif
		
		#if PRED_RESOURCE == true
			pred = new PRED;
			object_list->push_back(pred->get_object());
		#endif
		
	}
	
	void push_objects( M2MObjectList *object_list) {
		object_list->push_back(rgb->get_object());
		object_list->push_back(led->get_object());
		object_list->push_back(button->get_object());
		object_list->push_back(e_time->get_object());
		object_list->push_back(sense->get_object());
	}
	
public:
	SENSE *sense;
	RGB *rgb;
	BUTTON *button;
	LED *led;
	ELAPSED_T *e_time;
	PRED *pred;
};


#endif //__EASY_RESOURCES_H__