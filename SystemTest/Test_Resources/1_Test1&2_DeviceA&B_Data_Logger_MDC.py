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

token = "YDWYb01rNn4hHfMUIhfwV48QMVbJxeKWHamZB1kRyTFxec4MBXIlUNzCeMaMl4m6WKbrXKTRsAQS6lYVtF7bAUIfBmzeTgSPUrJ7"
Endpoint = '83a4d6e0-fcc4-469f-96d4-6f3a33ecd636'
#---------- Initialisations ----------#

x = mbed_connector_api.connector(token)									# Instantiation of the connector object

x.debug(True,'DEBUG')    # Set level of displayed messages in the terminal DEBUG INFO WARNING ERROR	

x.startLongPolling()	# Required for async_handling, can't do anything without this
#***************************************************#

time_resource = '/1/0/3'

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

#***************************************************#

def handler_notification(data):								#everytime a value changes, this is sent!
	print("---------------------------")
	print("handler_notification")				# contatins the resource that changed, and the new value! check async_Handler to decode base64
	print(data)
	payload = b64decode(data['notifications'][0]['payload'])
	print(payload)
	if(data['notifications'][0]['path'] == time_resource):
		add_time_to_csv(payload)
	else:
		print("no action for path")
		print data['notifications'][0]['path']
	print("+++++++++++++++++++++++++++")

#***************************************************#

x.setHandler('notifications', handler_notification)
r = x.putResourceSubscription(Endpoint,time_resource) # IF YOU DONT SUBSCRIBE HERE IT DOESNT COME IN TO THE HANDLER_NOTIFICATION!!!

x.postResource(Endpoint,time_resource,"START") # send the "Value" to the Resource over a POST request

#***************************************************#

time.sleep(2)

while True:
	time.sleep(10)

