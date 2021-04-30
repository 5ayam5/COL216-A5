#ifndef DRAM_H
#define DRAM_H

#include <bits/stdc++.h>
#include "MIPS_Core.hpp"

using namespace std;

struct MIPS_Core;

// struct which simulates the DRAM and memory management model
struct DRAM
{
private:
	// struct to store all information required by DRAM queue to execute the instruction
	struct QElem
	{
		bool id;
		int core, PCaddr, value, issueCycle, startCycle, colNum;

		QElem(int core, bool id, int PCaddr, int value, int colNum, int issueCycle, int startCycle = -1);
	};

public:
	// constants
	static const int MAX = 1 << 20, ROWS = 1 << 10, DRAM_MAX = 1 << 5;
	// DRAM delays
	int row_access_delay, col_access_delay, maxToProcess;
	// "dynamic" vars
	int currCore, currRow, rowBufferUpdates, delay, totPending, numProcessed, M, remainingCycles;
	// data stored in allocated memory
	vector<vector<int>> data;
	vector<int> buffer;
	// data structure to store info about requests sent to the DRAM, key is the row number and value is QElem
	vector<unordered_map<int, queue<QElem>>> DRAMbuffer;
	vector<MIPS_Core *> cores;
	vector<int> pendingCount, priority;
	unordered_map<int, pair<int, int>> forwarding;

	DRAM(int rowDelay, int colDelay);

	void simulateExecution(int m);
	void simulateCycle();
	void finishExecution();
	void finishCurrDRAM();
	void setNextDRAM(int nextCore, int nextRow);
	void popAndUpdate(queue<QElem> &Q, int &core, int &row, bool skip = false);
	void bufferUpdate(int core = -1, int row = -1);
	void printDRAMCompletion(int core, int PCaddr, int begin, int end, string action = "executed");
};

#endif
