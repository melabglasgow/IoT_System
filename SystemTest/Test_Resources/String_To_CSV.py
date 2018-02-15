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
import csv
import sys

data = sys.argv[1]

#===============================================================================#
#===============================================================================#

#-------------- csv to store ping data --------------#

f_time = open("manual_ping_data.csv", 'wt')

writer_time = csv.writer(f_time, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')

def add_time_to_csv(data):
	index_current = 1
	while ( (index_current < len(data)) & (index_current != 0) ):
		index_start = index_current
		index_current = data.find(',', index_current)
		print(', found at', index_current)
		input = data[index_start:index_current]
		print(input)
		writer_time.writerow([input])
		index_current += 1
	
#***************************************************#

add_time_to_csv(data)