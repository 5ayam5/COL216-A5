all: main

main: main.cpp DRAM.o MIPS_Core.o
	g++ -o main main.cpp DRAM.o MIPS_Core.o -std=c++17

MIPS_Core.o: MIPS_Core.cpp MIPS_Core.hpp
	g++ -c MIPS_Core.cpp -std=c++17

DRAM.o: DRAM.cpp DRAM.hpp
	g++ -c DRAM.cpp -std=c++17

clean:
	rm -f DRAM.o MIPS_Core.o main
