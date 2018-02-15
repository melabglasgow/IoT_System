import csv

class csv_man:
	
	def reset(self):
		#print["->reset()"]
		self.file = open(self.file_name+'.csv', 'rU')
		self.reader = csv.reader(self.file)
		if(self.garbage_flag):
			self.garbage_delete()
			
	def init(self,name,categ,convert_flag):
		print["->init()"]
		self.file_name = name
		self.garbage_flag = False
		self.row_start = 0
		self.garbage_count = 0
		self.row_s = 0
		self.col_s = 0
		
		self.categ = categ
		self.categ_size = len(self.categ)
		
		self.file = open(self.file_name+'.csv', 'rU')
		self.reader = csv.reader(self.file)
		first_line = self.reader.next()

		self.garbage_check(first_line)
		self.row_size()
		self.col_size()
		if(self.garbage_flag & convert_flag):
			self.convert()
		self.reset()

	def garbage_check(self,first_line):
		print["->garbage_check()"]
		self.reset()
		#print(first_line[0],"VS",self.categ[0])
		if(first_line[0] == self.categ[0]):
			self.garbage_flag = False
		else:
			self.garbage_flag = True
		print(self.garbage_flag)
	
	def garbage_delete(self):
		#	print["->garbage_delete()"]
		for x in range(6):
			self.reader.next()
	
	def line(self,n):
		#print["->line(n)",n]
		self.reset()
		for x in range(n):
			self.reader.next()
		return self.reader.next()
		
	def row_size(self):
		#print["->row_size()"]
		self.reset()
		self.row_s = len(self.reader.next())
		#print(self.row_s)
	
	def col_size(self):
		print["->col_size()"]
		self.reset()
		count = 0
		stop_count = False
		for row in self.reader:
			#print(len(row))
			empty = 0
			for n in range(self.row_s):
				if(row[n] == ''):
					empty = empty + 1
			if(empty == self.row_s):
				print("stopped before end")
				stop_count = True
			if not(stop_count):
				count = count+ 1
		self.col_s = count
		#print(self.col_s)
	
	def convert(self):
		print["->convert()"]
		self.converted = self.file_name+"_mod"
		f = open(self.converted+'.csv', 'wt')

		writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
		writer.writerow(self.categ)

		for i in range(self.col_s):
			writer.writerow(self.line(i))
		print self.converted
	
	def delete(self):
		print["->delete()"]
		self.deleted = self.file_name+"_del"
		f = open(self.deleted+'.csv', 'wt')
		writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
		writer.writerow(self.categ)
		
		self.reader.next()
		
		for n in range(1,self.col_s-2):
			line = self.reader.next()
			#print(line)
			test1 = False
			test1 = False
			check = False
			for m in range(1,self.categ_size):
				#print(str(line[m]))
				test1 = (str(line[m]) == "")
				test2 = (str(line[m]) == ".")
				test3 = (str(line[m]) == "#DIV/0!")
				if(test1 | test2 | test3):
					check = True
					#print(line[m],m)
			#print(test1,test2)
			#print check
			if(check):
				tre=0
				#print(test1,test2)
				#print("deleted")
			else:
				#print("written",n)
				writer.writerow(line)

	def clean(self):
		print["->clean()"]
		self.deleted = self.file_name+"_clean.csv"
		f = open(self.deleted, 'wt')
		writer = csv.writer(f, delimiter=',',quoting=csv.QUOTE_MINIMAL,lineterminator='\r')
		writer.writerow(self.categ)
		
		self.reader.next()
		
		for n in range(1,self.col_s-2):
			line = self.reader.next()
			for m in range(1,self.categ_size):
				if(line[m] == '.'):
					line[m] = ''
			writer.writerow(line)
