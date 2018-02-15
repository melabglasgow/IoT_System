import csv
from csv_manipulate import csv_man
			
#----------- import headers for csv file read and writes -----------#

my_csv = 'headers/categ_or.csv'
f = open(my_csv, 'rt')
reader_h = csv.reader(f)

categ_or = reader_h.next()
categ_or_len = len(categ_or)

my_csv = 'headers/categ_ds.csv'
f = open(my_csv, 'rt')
reader_h = csv.reader(f)

categ_ds = reader_h.next()
categ_ds_len = len(categ_or)

my_csv = 'headers/categ_cl.csv'
f = open(my_csv, 'rt')
reader_h = csv.reader(f)

categ_cl = reader_h.next()
categ_cl_len = len(categ_cl)

print ('categ_or',categ_or)
print ('categ_or_len',categ_or_len)
print ('categ_ds',categ_ds)
print ('categ_ds_len',categ_ds_len)

my_csv = 'climates_table.csv'
f_c = open(my_csv, 'rt')
reader_c = csv.reader(f_c)
climate =reader_c.next()
climate_len = len(climate)
line_c = climate[0]
curr_c = climate[1]
f_c = open(my_csv, 'rt')
reader_c = csv.reader(f_c)

#-------------------------------------------------------------------#

f = open("EXL_merge.csv", 'wt')

writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
writer.writerow(categ_ds)

for i in range(1,53):
	my_csv = "csv_data/"+str(i)+'_EXL'
	print my_csv
	
	obj = csv_man()
	obj.init(my_csv,categ_or,True)
	obj.reader.next()
	for n in range(1,obj.col_s):
		line = [str(i)]+ obj.reader.next()
		if( (i == 47) | (i == 48) ):
			for m in range(1,categ_or_len):
				if( (categ_or[m] == 'SEX') | ((m > 57) & (m < 65)) ):
				#if( (categ_or[m] == 'SEX') | (categ_or[m] == 'PCEC1') | (categ_or[m] == 'PCEC7')):
					prev = line[m]
					if( line[m] == '1'):
						line[m] = '0'
					elif( line[m] == '2'):
						line[m] = '1'
					else:
						line[m] = ''
					#print(prev,line[m],m)
		writer.writerow(line)
f.close()
#-------------------------------------------------------------------#

my_csv = "EXL_merge"
obj = csv_man()
obj.init(my_csv,categ_or,False)
obj.clean()

#-------------------------------------------------------------------#

f = open("EXL_merge_clean_climate.csv", 'wt')

writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
writer.writerow(categ_cl)

my_csv = 'EXL_merge_clean'

obj = csv_man()
obj.init(my_csv,categ_ds,False)
print("obj.row_s",obj.row_s)
print("obj.col_s",obj.col_s)
obj.reader.next()


for n in range(1,obj.col_s):
	line_raw = obj.reader.next()
	#print line_raw[0]
	#print curr_c
	if(line_raw[0] != line_c[0]):
		line_c = reader_c.next()
		curr_c = line_c[1]
		#print curr_c
	
	line = [curr_c]+ line_raw
	temp_cl = line[0]
	temp_ds = line[1]
	line[0] = temp_ds
	line[1] = temp_cl
	writer.writerow(line)
f.close()
#-------------------------------------------------------------------#

f = open("EXL_merge_clean_climate_time.csv", 'wt')

writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
writer.writerow(categ_cl)

my_csv = 'EXL_merge_clean_climate'

obj = csv_man()
obj.init(my_csv,categ_cl,False)
print("obj.row_s",obj.row_s)
print("obj.col_s",obj.col_s)
obj.reader.next()


for n in range(1,obj.col_s):
	line = obj.reader.next()
	#print line_raw[0]
	#print curr_c
	time = str(line[8])
	
	if not ((time == '') | (time == '.')):
		#print(obj.reader.line_num)
		if ":" not in time:
			#print("1",time)
			if "." in time:
				index = time.index('.')
				new_time = time[0:index]
				#print("1.1",obj.reader.line_num,index,time,new_time)
			else:
				size = len(time)
				if(size < 3):
					new_time = time
					#print("1.2.1",obj.reader.line_num,size,index,time,new_time)
				else:
					ammend = 0
					if(time[size-1] == ' '):
						ammend = 1
					index = len(time) - 2 - ammend
					new_time = time[0:index]
					#print("1.2.2",obj.reader.line_num,size,index,time,new_time)	
		else:
			#print("2",time)
			index = time.index(':')
			new_time = time[0:index]
			#print("2.1",index,new_time)
		line[8] = new_time
	writer.writerow(line)
	
f.close()