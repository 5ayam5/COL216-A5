#ifndef DRAM_H
#define DRAM_H

#include <bits/stdc++.h>
#include "MIPS_Core.hpp"

using namespace std;

struct MIPS_Core;

// struct which simulates the DRAM and memeory management model
struct DRAM
{
	// struct to store all information required by DRAM queue to execute the instruction
	struct QElem
	{
		bool id;
		int core, PCaddr, value, issueCycle, startCycle, remainingCycles;

		QElem(int core, bool id, int PCaddr, int value, int issueCycle, int startCycle = -1, int remainingCycles = -1);
	};

	// constants
	static const int MAX = 1 << 20, ROWS = 1 << 10, DRAM_MAX = 1 << 5;
	// DRAM delays
	int row_access_delay, col_access_delay;
	// "dynamic" vars
	int DRAMsize, currRow, currCol, rowBufferUpdates, delay;
	// data stored in allocated memory
	vector<vector<int>> data;
	vector<int> buffer;
	// data structure to store info about requests sent to the DRAM, key is the row number and value is QElem
	unordered_map<int, unordered_map<int, queue<QElem>>> DRAMbuffer;
	vector<MIPS_Core*> cores;

	DRAM(int rowDelay, int colDelay);

	void simulateExecution(int m);
	int simulateCycle();
	void finishExecution();
	void finishCurrDRAM(int nextRegister = 32);
	void setNextDRAM(int nextRow, int nextCol, int core = -1, int nextRegister = 32);
	bool popAndUpdate(queue<QElem> &Q, int &row, int &col, bool skip = false);
	void bufferUpdate(int row = -1, int col = -1);
	void printDRAMCompletion(int core, int PCaddr, int begin, int end, string action = "executed");
};

#endif
