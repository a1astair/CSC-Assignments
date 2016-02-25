
This folder contains:

./sudoku_to_CNF.py - A python program to convert a unsolved sudoku into a CNF

./sudoku_solver.py - A python program to convert a solved CNF into a solved sudoku and prints that result



How to Use

In this folder I have submitted two programs sudoku_to_CNF.py and sudoku_solver.py 
The first one sudoku_to_cnf.py converts the unsolved sudoku to CNF format. 
To run the program type ./sudoku_to_cnf.py 'puzzle.txt' where puzzle.txt would be the file containing the unsolved sudoku. 
After this step the program will ask you what you would like to name the converted CNF ex. 
'Please input filename for the CNF formula: "formula.cnf"' formula.cnf would be the filename of the converted CNF.


The second step would be to run the CNF through miniSAT ex. minisat 'formula1.cnf' 'answer1.cnf' Formula1.cnf is the CNF file and answer1.cnf will be the file containing the solved CNf.


The third step is to run sudoku_solver.py. This program takes the solved CNF and converts that into a solved sudoku and displays that answer on the screen ex. /sudoku_solver.py answer1.cnf. 
This would take in answer1.cnf and then display the solved sudoku.



Summary

This project was a great tool in learning how CNF's work and how to use miniSAT properly. 
The programs I designed work on all sudoku's even unsatisfiable one's because i just print out the fact that it is unsatisfiable. 
I checked a lot of edge cases and it seems to work on them all. 
It can read in all the formats displayed in the project PDF. 
I am very happy with the result of this project. 
