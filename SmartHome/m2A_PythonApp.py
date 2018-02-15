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

token = "YwV48QMVbJxeKWHamZB1kRyTFxec4MBXIlUNzCeMaMl4m6WKbrXKTRsAQS6lYVtF7bAUIfBmzeTgSPUrJ7"
Endpoint = '83a4d7e0-fcc4-469f-97d4-6g3a33ecd636'

#---------- Initialisations ----------#

app = mbed_connector_api.connector(token)									# Instantiation of the connector object

app.debug(False,'DEBUG')    # Set level of displayed messages in the terminal DEBUG INFO WARNING ERROR	

app.startLongPolling()	# Required for async_handling, can't do anything without this

#***************************************************#

resource = {'led_status': '/11/0/1', 'button_count': '/12/0/1', 'rgb_color': '/13/0/1', 'sense_gyro': '/14/0/1', 'sense_acce': '/14/0/2', 'sense_magn': '/14/0/3', 'sense_humi': '/14/0/4', 'sense_pres': '/14/0/5', 'sense_temp': '/14/0/6', 'sense_motion': '/14/0/7', 'etime_time': '/15/0/1', 'pred_curr': '/16/0/1', 'pred_ash': '/16/0/2', }

myList = resource.items()

def async_print(r,description):				# Extracting information from the async_result object: a description is added to identify the command executed.
	sys.stdout.write("---------------------------\n")
	if r.isDone():																# The ones that DON'T require async handling will set this to true.
		if not r.error:															# This error depends on HTTP status code (example 200, error = False)
			print( "[%s]: Status code is %s" % (description,r.status_code) )	# HTTP status code
			print( "[%s]: The value of r is %s" % (description,r.result) )
		else:																	# IF NOT : if data.content is a string, r.result = data.content, elif its a integer, CRY
			print( "Error : %s" % (r.error.error) )
	else:
		print( "[%s]: async handling required!" % description )					# The command requires async handling
		print( "[%s]: The raw data of r is %s" % (description,r.raw_data) )		# Print the raw data, in the case of getResourceValue the async ID is found here
	sys.stdout.write("---------------------------\n")

def handler_notification(data):				#everytime a value changes, this is sent!
	path =  data['notifications'][0]['path']
	payload = b64decode(data['notifications'][0]['payload'])
	print(path,len(data),len(payload))
	
	if(data['notifications'][0]['path'] == resource['button_count']):
		print 'button_count:' + payload
	elif(data['notifications'][0]['path'] == resource['rgb_color']):
		print 'rgb_color:' + payload
	elif(data['notifications'] [0]['path'] == resource['sense_gyro']):
		print 'sense_gyro:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_acce']):
		print 'sense_acce:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_magn']):
		print 'sense_magn:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_humi']):
		print 'sense_humi:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_pres']):
		print 'sense_pres:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_temp']):
		print 'sense_temp:' + payload
	elif(data['notifications'][0]['path'] == resource['sense_motion']):
		print 'sense_motion:' + payload
		r = app.postResource(Endpoint,resource['etime_time'],"RESET")
		async_print(r,"putResourceValue")
	elif(data['notifications'][0]['path'] == resource['etime_time']):
		print 'etime_time:' + payload
	elif(data['notifications'][0]['path'] == resource['pred_curr']):
		print 'pred_curr:' + payload
	elif(data['notifications'][0]['path'] == resource['pred_ash']):
		print 'pred_ash:' + payload
	
#***************************************************#

#---------- Endpoint information retrieve ----------#

r = app.getEndpoints() 			# returns a  list of all endpoints on your domain
async_print(r,"getEndpoints")	# prints the async_request data
r = app.getResources(Endpoint) 	# returns all resources of an endpoint
async_print(r,"getResources")	# prints the async_request data

#---------------------------------------------------#
#---------------------------------------------------#

for i in range (0,len(resource)):
	res = str(myList[i][1])
	print res
	r = app.putResourceSubscription(Endpoint,res)
	async_print(r,"putResourceSubscription(" +res+ ")")

app.setHandler('notifications', handler_notification)

#***************************************************#
r = app.putResourceValue(Endpoint,resource['led_status'],"ON")
async_print(r,"putResourceValue")
while True:
	r = app.putResourceValue(Endpoint,resource['rgb_color'],"OFF")
	async_print(r,"putResourceValue")
	time.sleep(30)
	
