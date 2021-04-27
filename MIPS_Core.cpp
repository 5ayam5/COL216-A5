#include <bits/stdc++.h>
#include <boost/tokenizer.hpp>
#include "MIPS_Core.hpp"

using namespace std;

int MIPS_Core::clockCycles = 0, MIPS_Core::instructionsCount = 0;
DRAM *MIPS_Core::dram;

// constructor to initialise the instruction set
MIPS_Core::MIPS_Core(ifstream &file, int id)
{
	this->id = id;

	instructions = {{"add", &MIPS_Core::add}, {"sub", &MIPS_Core::sub}, {"mul", &MIPS_Core::mul}, {"beq", &MIPS_Core::beq}, {"bne", &MIPS_Core::bne}, {"slt", &MIPS_Core::slt}, {"j", &MIPS_Core::j}, {"lw", &MIPS_Core::lw}, {"sw", &MIPS_Core::sw}, {"addi", &MIPS_Core::addi}};

	for (int i = 0; i < 32; ++i)
		registerMap["$" + to_string(i)] = i;
	registerMap["$zero"] = 0;
	registerMap["$at"] = 1;
	registerMap["$v0"] = 2;
	registerMap["$v1"] = 3;
	for (int i = 0; i < 4; ++i)
		registerMap["$a" + to_string(i)] = i + 4;
	for (int i = 0; i < 8; ++i)
		registerMap["$t" + to_string(i)] = i + 8, registerMap["$s" + to_string(i)] = i + 16;
	registerMap["$t8"] = 24;
	registerMap["$t9"] = 25;
	registerMap["$k0"] = 26;
	registerMap["$k1"] = 27;
	registerMap["$gp"] = 28;
	registerMap["$sp"] = 29;
	registerMap["$s8"] = 30;
	registerMap["$ra"] = 31;

	constructCommands(file);
}

// perform add immediate operation
int MIPS_Core::addi(string r1, string r2, string num)
{
	if (!checkRegisters({r1, r2}) || registerMap[r1] == 0)
		return 1;
	try
	{
		if (registersAddrDRAM[registerMap[r2]] != make_pair(-1, -1))
			return -registerMap[r2] - 1;
		registers[registerMap[r1]] = registers[registerMap[r2]] + stoi(num);
		registersAddrDRAM[registerMap[r1]] = {-1, -1};
		PCnext = PCcurr + 1;
		return 0;
	}
	catch (exception &e)
	{
		return 4;
	}
}

// perform add operation
int MIPS_Core::add(string r1, string r2, string r3)
{
	return op(r1, r2, r3, [&](int a, int b) { return a + b; });
}

// perform subtraction operation
int MIPS_Core::sub(string r1, string r2, string r3)
{
	return op(r1, r2, r3, [&](int a, int b) { return a - b; });
}

// perform multiplication operation
int MIPS_Core::mul(string r1, string r2, string r3)
{
	return op(r1, r2, r3, [&](int a, int b) { return a * b; });
}

// implements slt operation
int MIPS_Core::slt(string r1, string r2, string r3)
{
	return op(r1, r2, r3, [&](int a, int b) { return a < b; });
}

// perform the operation
int MIPS_Core::op(string r1, string r2, string r3, function<int(int, int)> operation)
{
	if (!checkRegisters({r1, r2, r3}) || registerMap[r1] == 0)
		return 1;
	if (registersAddrDRAM[registerMap[r2]] != make_pair(-1, -1))
		return -registerMap[r2] - 1;
	if (registersAddrDRAM[registerMap[r3]] != make_pair(-1, -1))
		return -registerMap[r3] - 1;
	registers[registerMap[r1]] = operation(registers[registerMap[r2]], registers[registerMap[r3]]);
	registersAddrDRAM[registerMap[r1]] = {-1, -1};
	PCnext = PCcurr + 1;
	return 0;
}

// perform the beq operation
int MIPS_Core::beq(string r1, string r2, string label)
{
	return bOP(r1, r2, label, [](int a, int b) { return a == b; });
}

// perform the bne operation
int MIPS_Core::bne(string r1, string r2, string label)
{
	return bOP(r1, r2, label, [](int a, int b) { return a != b; });
}

// implements beq and bne by taking the comparator
int MIPS_Core::bOP(string r1, string r2, string label, function<bool(int, int)> comp)
{
	if (!checkLabel(label))
		return 4;
	if (labels.find(label) == labels.end() || labels[label] == -1)
		return 2;
	if (!checkRegisters({r1, r2}))
		return 1;
	if (registersAddrDRAM[registerMap[r1]] != make_pair(-1, -1))
		return -registerMap[r1] - 1;
	if (registersAddrDRAM[registerMap[r2]] != make_pair(-1, -1))
		return -registerMap[r2] - 1;
	PCnext = comp(registers[registerMap[r1]], registers[registerMap[r2]]) ? labels[label] : PCcurr + 1;
	return 0;
}

// perform the jump operation
int MIPS_Core::j(string label, string unused1, string unused2)
{
	if (!checkLabel(label))
		return 4;
	if (labels.find(label) == labels.end() || labels[label] == -1)
		return 2;
	PCnext = labels[label];
	return 0;
}

// perform load word operation
int MIPS_Core::lw(string r, string location, string unused1)
{
	if (!checkRegister(r) || registerMap[r] == 0)
		return 1;
	auto address = locateAddress(location);
	if (!address.first)
		return address.second;

	PCnext = PCcurr + 1;
	if (address.second == lastAddr.first)
	{
		registers[registerMap[r]] = lastAddr.second;
		registersAddrDRAM[registerMap[r]] = {-1, -1};
		return 0;
	}

	if (dram->DRAMsize == DRAM::DRAM_MAX)
		return -33;
	isDRAM = true;
	dram->DRAMbuffer[id][address.second / DRAM::ROWS][(address.second % DRAM::ROWS) / 4].push({id, 1, PCcurr, registerMap[r], clockCycles});
	registersAddrDRAM[registerMap[r]] = {clockCycles, address.second};
	++dram->pendingCount[id], ++dram->DRAMsize, ++dram->totPending, --instructionsCount;
	return 0;
}

// perform store word operation
int MIPS_Core::sw(string r, string location, string unused1)
{
	if (!checkRegister(r))
		return 1;
	auto address = locateAddress(location);
	if (!address.first)
		return address.second;
	if (registersAddrDRAM[registerMap[r]] != make_pair(-1, -1))
		return -registerMap[r] - 1;

	if (dram->DRAMsize == DRAM::DRAM_MAX)
		return -33;
	lastAddr = {address.second, registers[registerMap[r]]};
	isDRAM = true;
	dram->DRAMbuffer[id][address.second / DRAM::ROWS][(address.second % DRAM::ROWS) / 4].push({id, 0, PCcurr, registers[registerMap[r]], clockCycles});
	++dram->pendingCount[id], ++dram->DRAMsize, ++dram->totPending, --instructionsCount;
	PCnext = PCcurr + 1;
	return 0;
}

// parse the address and return the actual address or error
pair<bool, int> MIPS_Core::locateAddress(string location)
{
	if (location.back() == ')')
	{
		try
		{
			int lparen = location.find('('), offset = stoi(lparen == 0 ? "0" : location.substr(0, lparen));
			string reg = location.substr(lparen + 1);
			reg.pop_back();
			if (!checkRegister(reg))
				return {false, 3};
			if (registersAddrDRAM[registerMap[reg]] != make_pair(-1, -1))
				return {false, -registerMap[reg] - 1};
			int address = registers[registerMap[reg]] + offset;
			if (address % 4 || address >= DRAM::MAX / MAX_CORES)
				return {false, 3};
			return {true, address + id * DRAM::MAX / MAX_CORES};
		}
		catch (exception &e)
		{
			return {false, 4};
		}
	}
	try
	{
		int address = stoi(location);
		if (address % 4 || address >= DRAM::MAX / MAX_CORES)
			return {false, 3};
		return {true, address + id * DRAM::MAX / MAX_CORES};
	}
	catch (exception &e)
	{
		return {false, 4};
	}
}

// checks if label is valid
inline bool MIPS_Core::checkLabel(string str)
{
	return str.size() > 0 && isalpha(str[0]) && all_of(++str.begin(), str.end(), [](char c) { return (bool)isalnum(c); }) && instructions.find(str) == instructions.end();
}

// checks if the register is a valid one
inline bool MIPS_Core::checkRegister(string r)
{
	return registerMap.find(r) != registerMap.end();
}

// checks if all of the registers are valid or not
bool MIPS_Core::checkRegisters(vector<string> regs)
{
	return all_of(regs.begin(), regs.end(), [&](string r) { return checkRegister(r); });
}

// parse the command assuming correctly formatted MIPS instruction (or label)
void MIPS_Core::parseCommand(string line)
{
	// strip until before the comment begins
	line = line.substr(0, line.find('#'));
	vector<string> command;
	boost::tokenizer<boost::char_separator<char>> tokens(line, boost::char_separator<char>(", \t"));
	for (auto &s : tokens)
		command.push_back(s);
	// empty line or a comment only line
	if (command.empty())
		return;
	else if (command.size() == 1)
	{
		string label = command[0].back() == ':' ? command[0].substr(0, command[0].size() - 1) : "?";
		if (labels.find(label) == labels.end())
			labels[label] = commands.size();
		else
			labels[label] = -1;
		command.clear();
	}
	else if (command[0].back() == ':')
	{
		string label = command[0].substr(0, command[0].size() - 1);
		if (labels.find(label) == labels.end())
			labels[label] = commands.size();
		else
			labels[label] = -1;
		command = vector<string>(command.begin() + 1, command.end());
	}
	else if (command[0].find(':') != string::npos)
	{
		int idx = command[0].find(':');
		string label = command[0].substr(0, idx);
		if (labels.find(label) == labels.end())
			labels[label] = commands.size();
		else
			labels[label] = -1;
		command[0] = command[0].substr(idx + 1);
	}
	else if (command[1][0] == ':')
	{
		if (labels.find(command[0]) == labels.end())
			labels[command[0]] = commands.size();
		else
			labels[command[0]] = -1;
		command[1] = command[1].substr(1);
		if (command[1] == "")
			command.erase(command.begin(), command.begin() + 2);
		else
			command.erase(command.begin(), command.begin() + 1);
	}
	if (command.empty())
		return;
	if (command.size() > 4)
		for (int i = 4; i < (int)command.size(); ++i)
			command[3] += " " + command[i];
	command.resize(4);
	commands.push_back(command);
}

// construct the commands vector from the input file
void MIPS_Core::constructCommands(ifstream &file)
{
	string line;
	while (getline(file, line))
		parseCommand(line);
	file.close();
}

// execute the commands sequentially
int MIPS_Core::executeCommand()
{
	if (commands.size() >= MAX / 4)
		return 5;
	if (PCcurr >= (int)commands.size())
		return 0;
	isDRAM = false;
	vector<string> &command = commands[PCcurr];
	if (instructions.find(command[0]) == instructions.end())
	{
		handleError(4);
		return 4;
	}
	int ret = instructions[command[0]](*this, command[1], command[2], command[3]);
	if (ret > 0)
	{
		handleError(ret);
		return ret;
	}
	if (ret != 0)
		return ret;
	PCcurr = PCnext, ++instructionsCount;
	printCycleExecution(command, PCcurr);
	return 0;
}

// print cycle info
void MIPS_Core::printCycleExecution(vector<string> &command, int PCaddr)
{
	cout << "(Core " << id << ") ";
	if (isDRAM)
		cout << clockCycles << ": (DRAM call queued) ";
	else
		cout << clockCycles << ": " << (command[0] == "lw" ? "(used forwarding) " : "");
	cout << "(PC address " << PCaddr << ") ";
	for (auto &s : command)
		cout << s << ' ';
	cout << '\n';
	printRegisters();
	cout << "\n";
}

// print the register data in hexadecimal
void MIPS_Core::printRegisters()
{
	cout << hex;
	for (int i = 0; i < 32; ++i)
		cout << registers[i] << ' ';
	cout << dec << "\n";
}

/*
	handle all exit codes:
	0: correct execution
	1: register provided is incorrect
	2: invalid label
	3: unaligned or invalid address
	4: syntax error
	5: commands exceed memory limit
*/
void MIPS_Core::handleError(int code)
{
	switch (code)
	{
	case 1:
		cerr << "Invalid register provided or syntax error in providing register\n";
		break;
	case 2:
		cerr << "Label used not defined or defined too many times\n";
		break;
	case 3:
		cerr << "Unaligned or invalid memory address specified\n";
		break;
	case 4:
		cerr << "Syntax error encountered\n";
		break;
	case 5:
		cerr << "Memory limit exceeded\n";
		break;
	default:
		return;
	}
	cerr << "Error encountered in core " << id << " at:\n";
	for (auto &s : commands[PCcurr])
		cerr << s << ' ';
	cerr << '\n';
}

// initialize variables before executing commands
void MIPS_Core::initVars()
{
	if (commands.size() >= MAX / 4)
	{
		handleError(5);
		return;
	}
	fill_n(registers, 32, 0);
	fill_n(registersAddrDRAM, 32, make_pair(-1, -1));
	lastAddr = {-1, -1};
}
