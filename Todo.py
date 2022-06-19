from os import system
import time, datetime
from pathlib import Path

try:
	print("Scanning files...")
	
	file_types = ["c", "cpp", "h", "hpp", "shader"]

	while True:
		files = []
		todo_list = []
		total_lines = 0
		error_files = []
		
		start_time = time.time()
		
		found_files=[]
		found_files.extend(Path('.').glob('**/*.*'))
		for file in found_files:
			if (file.suffix[1:].rstrip()) in file_types:
				files.append(file)
			
		elapsed_time = time.time() - start_time
		
		start_time = time.time()
		
		for file in files:
			try:
				fh = open(file)
			
			except IOError:
				error_files.append(file)
				
			if not fh.closed:
				line_number = 0
				for line in fh:
					line_number += 1
					if "//TODO" in line:
						index = line.find("//TODO")
						line = line[index:].rstrip()
						line = '{0: <50}'.format(line) + "{" + str(file) + " @ " + str(line_number) + "}"
						todo_list.append(line)
				fh.close()
			
			total_lines += line_number
			
		elapsed_time2 = time.time() - start_time
		
		system('cls')
		print(str(round(elapsed_time * 1000) / 1000) + " seconds of scanning")
		print(str(round(elapsed_time2 * 1000) / 1000) + " seconds of processing")
		print(str(len(files)) + " file(s) found")
		print(str(len(error_files)) + " error(s) occurred")
		print(str(total_lines) + " lines of code\n")
		for todo in todo_list:
			print(todo)
			
		time.sleep(.2)
		
except KeyboardInterrupt:
	print(" ")