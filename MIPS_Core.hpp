#ifndef MIPS_Core_H
#define MIPS_Core_H

#include <bits/stdc++.h>
#include "DRAM.hpp"

using namespace std;

struct DRAM;

// struct to store the registers and the functions to be executed
struct MIPS_Core
{
	// maximum size of instruction set
	static const int MAX = 1 << 8, MAX_CORES = 1 << 4;
	// instruction set
	unordered_map<string, function<int(MIPS_Core &, string, string, string)>> instructions;
	// mapping names to a unique number
	unordered_map<string, int> registerMap, labels;
	// vector to store the commands in input program
	vector<vector<string>> commands;
	// "dynamic" vars
	int registers[32], PCcurr, PCnext, id, forwardedVal, DRAMsize;
	static int clockCycles, instructionsCount;
	pair<int, int> registersAddrDRAM[32];
	bool isDRAM, writePending, writePortBusy, isForwarding;
	static DRAM *dram;

	MIPS_Core(ifstream &file, int id);

	int addi(string r1, string r2, string num);
	int add(string r1, string r2, string num);
	int sub(string r1, string r2, string num);
	int mul(string r1, string r2, string num);
	int slt(string r1, string r2, string num);
	int op(string r1, string r2, string r3, function<int(int, int)> operation);
	int beq(string r1, string r2, string label);
	int bne(string r1, string r2, string label);
	int bOP(string r1, string r2, string label, function<bool(int, int)> comp);
	int j(string label, string unused1 = "", string unused2 = "");
	int lw(string r, string location, string unused1 = "");
	int sw(string r, string location, string unused1 = "");
	pair<bool, int> locateAddress(string location);
	inline bool checkLabel(string str);
	inline bool checkRegister(string r);
	bool checkRegisters(vector<string> regs);
	void parseCommand(string line);
	void constructCommands(ifstream &file);
	int executeCommand();
	void printCycleExecution(vector<string> &command, int PCaddr);
	void printRegisters();
	void handleError(int code);
	void initVars();
};

#endif
