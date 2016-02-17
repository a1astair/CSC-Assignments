#!/usr/bin/env python
import sys

def main():
	if len(sys.argv) == 1:
		print "Usage: ./sudoku_solver.py 'MiniSAT output file'"
		return
	else:
		arg1 = sys.argv[1]
		readin(arg1)

def readin(arg1):

	with open(arg1) as f:
		content = f.readlines()
		#split the second line of the file
		if (content[0] == 'UNSAT\n'):
			print "UNSATISFIABLE"
			return
		for i in content[1].split():
			n = int(i)
			if (n < 1):
				continue
			#Decode the numbers
			n = n - 1
			x = n//81
			y = (n-81*x)//9
			z = (n-81*x-9*y)+1
			print z,
			if (y == 8 and x != 8):
				print "" 

if __name__ == "__main__":
	main()

