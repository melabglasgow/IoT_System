'''
/*
 * Copyright 2010-2017 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
 '''

from AWSIoTPythonSDK.MQTTLib import AWSIoTMQTTClient
import logging
import time
import argparse
import csv
import mbed_connector_api
import sys
from base64 import standard_b64decode as b64decode
import json
import sys

#===============================================================================#
#===============================================================================#

#-------------- argument parsing for mode. --------------#

if(len(sys.argv) == 2):
	arg = sys.argv[1]
else:
	print('-------------------\n please provide the mode of the program as 1 argument.\n-------------------')
	sys.exit(0)

mode = 0
if(arg == "ble"):
	mode = arg
	print('-------------------\n ble mode\n-------------------')
	
elif(arg == "wifi"):
	mode = arg
	print('-------------------\n wifi mode\n-------------------')
else:
	print('-------------------\n wrong mode, possible modes [ble,wifi]\n-------------------')
	sys.exit(0)
time.sleep(2)


token = "YDWYb01rNn4hHfMUIhfwV48QMVbJxeKWHamZB1kRyTFxec4MBXIlUNzCeMaMl4m6WKbrXKTRsAQS6lYVtF7bAUIfBmzeTgSPUrJ7"

if(mode == "ble"):
	Endpoint = '21ba7a79-a7b6-418f-823e-bb89972028e7'
if(mode == "wifi"):
	Endpoint = '83a4d6e0-fcc4-469f-96d4-6f3a33ecd636'

#===============================================================================#
#===============================================================================#

#-------------- csv to store ping data --------------#

f_time = open("ping_data.csv", 'wt')

writer_time = csv.writer(f_time, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')

def add_time_to_csv(data):
	index_current = 0
	while (index_current < len(data)):
		index_start = index_current
		index_current = data.find(',', index_current)
		print(', found at', index_current)
		input = data[index_start:index_current]
		print(input)
		writer_time.writerow([input])
		index_current += 1
	
#***************************************************#

#===============================================================================#
#===============================================================================#

#---------- Initialisations ----------#

x = mbed_connector_api.connector(token)									# Instantiation of the connector object

x.debug(False,'DEBUG')    # Set level of displayed messages in the terminal DEBUG INFO WARNING ERROR	

x.startLongPolling()	# Required for async_handling, can't do anything without this
#***************************************************#
if(mode == "wifi"):
	Resource_out	= '/1/0/1'
	Resource_in		= '/1/0/2'
	Resource_time	= '/1/0/3'

if(mode == "ble"):
	Resource_out	= '/a000/0/a001'
	Resource_in		= '/a000/0/a002'
	Resource_start	= '/a000/0/a003'
	


def handler_notification_ble(data):								#everytime a value changes, this is sent!
	path =  data['notifications'][0]['path']
	payload = b64decode(data['notifications'][0]['payload'])
	length = len(payload) # plus the '\0' at the end!
	print(path,length)
	#print(payload[1],payload[2],payload[3])
	if(data['notifications'][0]['path'] == Resource_out):
		if(length > 3):
			strs = "0"
			for i in range(1, (length) ):
				strs = strs + str(i%10)
			print(strs,len(strs))
			x.putResourceValue(Endpoint,Resource_in,strs)
		else:
			x.putResourceValue(Endpoint,Resource_in,int(ord(payload[0])))
	elif(data['notifications'][0]['path'] == Resource_time):
		add_time_to_csv(payload)


def handler_notification_wifi(data):								#everytime a value changes, this is sent!
	path =  data['notifications'][0]['path']
	payload = b64decode(data['notifications'][0]['payload'])
	print(path,len(payload))
	if(data['notifications'][0]['path'] == Resource_out):
		x.putResourceValue(Endpoint,Resource_in,payload)
	elif(data['notifications'][0]['path'] == Resource_time):
		add_time_to_csv(payload)

#***************************************************#
if(mode == "wifi"):
	x.setHandler('notifications', handler_notification_wifi)
if(mode == "ble"):
	x.setHandler('notifications', handler_notification_ble)

r = x.putResourceSubscription(Endpoint,Resource_out)

if(mode == "ble"):
	x.putResourceValue(Endpoint,Resource_start,"START")

if(mode == "wifi"):
	r = x.putResourceSubscription(Endpoint,Resource_time)
	x.postResource(Endpoint,Resource_time,"START")

#***************************************************#

while True:
	time.sleep(15)

