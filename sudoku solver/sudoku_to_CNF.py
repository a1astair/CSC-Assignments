#!/usr/bin/env python
import sys

def main():
	if len(sys.argv) == 1:
		print "Usage: ./sudoku_to_CNF.py 'Puzzle file'"
		return
	else:
		arg1 = sys.argv[1]
		readin(arg1)

def readin(arg1):
	#ask user to name the file they want to make the CNF in
	filename = raw_input("Please input filename for the CNF formula: ")
	f1 = open(filename, 'w+')

	#print header in file
	print >>f1, "c", arg1

	clauses = parse(arg1)			
	
	print >>f1, "p cnf 729", (8829+len(clauses))

	for i in range(len(clauses)):
		print >>f1, clauses[i]

	rules(f1)

def parse(arg1):
	i = 0
	j = 0
	clauses = []

	with open(arg1) as f:
		content = f.readlines()
		for line in content:
			line = line.strip()
			for char in line:
				if (char.isdigit()):
					l = int(char)
					if (l >= 1 and l <= 9):
						#encoding
						clauses.append(str(81*i+9*j+l) + ' 0')
					j+=1
					if (j == 9):
						j=0
						i+=1
					if (i == 9):
						return clauses

	print "Incorrect number of entries"
	print "Read in", len(clauses)
	print "Expected 81. The remaining entries will be assumed empty"
	return clauses

def rules(f1):
	counter = 0
	#At least one number in each spot
	for i in range(0, 9):
		for j in range(0, 9):
			for k in range (0, 9):
				print >>f1, (81*i+9*j+k+1),	
			print >>f1, "0"
			
	#Each number appears at most once in each row
	for j in range(0, 9):
		for k in range(0, 9):
			for i in range (0, 8):
				for l in range((i+1), 9):
					print >>f1, (-1*(81*i+9*j+k+1)), (-1*(81*l+9*j+k+1)), "0"

	#Each number appears at most once in each column
	for i in range(0, 9):
		for k in range(0, 9):
			for j in range (0, 8):
				for l in range((j+1), 9):
					print >>f1, (-1*(81*i+9*j+k+1)), (-1*(81*i+9*l+k+1)), "0"

	#Each number appears at most once in each 3*3 sub-grid
	for z in range(0, 9):
		for i in range(0, 3):
			for j in range(0, 3):
				for x in range(0, 3):
					for y in range(0, 3):
						for k in range((y+1), 3):
							print >>f1, (-1*(81*(3*i+x)+9*(3*j+y)+z+1)), (-1*(81*(3*i+x)+9*(3*j+k)+z+1)), "0"

	for z in range(0, 9):
		for i in range(0, 3):
			for j in range(0, 3):
				for x in range(0, 3):
					for y in range(0, 3):
						for k in range((x+1), 3):
							for l in range(0, 3):
								print >>f1, (-1*(81*(3*i+x)+9*(3*j+y)+z+1)), (-1*(81*(3*i+k)+9*(3*j+l)+z+1)), "0"

	f1.close()

if __name__ == "__main__":
	main()

