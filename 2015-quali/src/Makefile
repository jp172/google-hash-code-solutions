all:
	g++ -m64 -O2 -Wall -std=gnu++11 -g -o solve main.cpp -I /Library/gurobi810/mac64/include/ -L /Library/gurobi810/mac64/lib/ -lgurobi_c++ -lgurobi81 -lm
par:
	g++-7 -m64 -O2 -Wall -std=gnu++11 -fopenmp -g -o solve main.cpp
lin:
	g++ -O2 -std=gnu++11 -m64 -fopenmp -g -o solve main.cpp -I/opt/gurobi/linux64/include/ -L/opt/gurobi/linux64/lib/ -lgurobi_c++ -lgurobi70 -lm
