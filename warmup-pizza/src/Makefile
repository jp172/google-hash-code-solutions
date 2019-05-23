all:
	g++ -m64 -O2 -Wall -std=gnu++11 -g -o solve main.cpp -I /Library/gurobi752/mac64/include/ -L /Library/gurobi752/mac64/lib/ -lgurobi_c++ -lgurobi75 -lm
lin:
	g++ -O2 -std=gnu++11 -m64 -fopenmp -g -o solve main.cpp -I/opt/gurobi/linux64/include/ -L/opt/gurobi/linux64/lib/ -lgurobi_c++ -lgurobi70 -lm
