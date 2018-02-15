import csv
from csv_manipulate import csv_man

import sys

if not (len(sys.argv) == 2):
	print "Please provide the name of the excel from which to delete rows with empty fields (without the file extension)"		
	sys.exit()
	
#----------- import headers for csv file read and writes -----------#

my_csv = sys.argv[1]+'.csv'
f = open(my_csv, 'rU')
reader_h = csv.reader(f)

categ_del = reader_h.next()
categ_del_len = len(categ_del)
f.close()

print ('categ_del',categ_del)
print ('categ_del_len',categ_del_len)

#-------------------------------------------------------------------#

my_csv = sys.argv[1]
print my_csv
obj = csv_man()
obj.init(my_csv,categ_del,False)
obj.delete()