import csv
from csv_manipulate import csv_man
import sys

if not (len(sys.argv) == 2):
	print "Please provide the name of the excel from which to select (without the file extension)"
	sys.exit()
#----------- import headers for csv file read and writes -----------#

#----------- import headers for csv file read and writes -----------#

my_csv = sys.argv[1]+'.csv'
f = open(my_csv, 'rU')
reader_h = csv.reader(f)

categ_ds = reader_h.next()
categ_ds_len = len(categ_ds)
f.close()

print ('categ_ds',categ_ds)
print ('categ_ds_len',categ_ds_len)

my_csv = 'headers/categ_sel.csv'
f = open(my_csv, 'rt')
reader_h = csv.reader(f)

categ_sel = reader_h.next()
categ_sel_len = len(categ_sel)

print ('categ_ds',categ_ds)
print ('categ_ds_len',categ_ds_len)
print ('categ_sel',categ_sel)
print ('categ_sel_len',categ_sel_len)

#-------------------------------------------------------------------#

f = open("EXL_selected.csv", 'wt')

writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
writer.writerow(categ_sel)

my_csv = sys.argv[1]
print my_csv
obj = csv_man()
obj.init(my_csv,categ_ds,False)
print("obj.row_s",obj.row_s)
print("obj.col_s",obj.col_s)
obj.reader.next()

for n in range(1,obj.col_s-1):
	line = obj.reader.next()
	r = 1
	line_r = [line[0]]
	#print(line_r)
	for m in range(1,categ_ds_len):
		#print("m="+str(m))
		#print("r="+str(r))
		if( (categ_ds[m] == categ_sel[r]) ):
			#print(categ_ds[m],categ_sel[r])
			line_r = line_r + [line[m]]
			#print(line_r)
			if(r < (categ_sel_len-1)):
				r = r + 1
	#print(line_r)
	writer.writerow(line_r)