#include "DRAM.hpp"

/**
 * initialise the queue element
 * @param id 0 for sw and 1 for lw
 * @param PCaddr stores the PC address of corresponding instruction
 * @param value content to be stored (for sw) / register id (for lw)
 * @param issueCycle the cycle in which the request was issued
 * @param startCycle the cycle when the instruction began
 * @param remainingCycles number of cycles pending to finish execution of request
*/
DRAM::QElem::QElem(int core, bool id, int PCaddr, int value, int issueCycle, int startCycle, int remainingCycles)
{
	this->id = id;
	this->core = core;
	this->PCaddr = PCaddr;
	this->value = value;
	this->issueCycle = issueCycle;
	this->startCycle = startCycle;
	this->remainingCycles = remainingCycles;
}

DRAM::DRAM(int rowDelay, int colDelay)
{
	row_access_delay = rowDelay;
	col_access_delay = colDelay;
	maxToProcess = rowDelay / colDelay;
	data.assign(ROWS, vector<int>(ROWS >> 2, 0));
}

// simulate complete execution
void DRAM::simulateExecution(int m)
{
	M = m;
	MIPS_Core::clockCycles = 1, MIPS_Core::instructionsCount = 0;
	currCore = currRow = currCol = -1;
	totPending = 0, numProcessed = 0;
	pendingCount.assign(cores.size(), 0);
	priority.assign(cores.size(), -1);
	DRAMbuffer.assign(cores.size(), unordered_map<int, unordered_map<int, queue<QElem>>>());

	for (auto &core: cores)
		core->initVars();

	while (MIPS_Core::clockCycles <= M && simulateCycle() == 0)
		continue;

	finishExecution();
}

// simulate a cycle execution
int DRAM::simulateCycle()
{
	for (int core = 0; core < (int)cores.size(); ++core)
	{
		int ret = cores[core]->executeCommand();
		if (ret > 0)
			return ret;
		priority[core] = -ret - 1;
	}

	if (totPending != 0)
	{
		// first lw/sw operation after DRAM_buffer emptied
		if (currCore == -1)
		{
			int nextCore = find_if(pendingCount.begin(), pendingCount.end(), [](int &c) { return c != 0; }) - pendingCount.begin();
			setNextDRAM(nextCore, DRAMbuffer[nextCore].begin()->first, DRAMbuffer[nextCore].begin()->second.begin()->first);
		}
		else if (--DRAMbuffer[currCore][currRow][currCol].front().remainingCycles == 0)
			finishCurrDRAM();
	}

	++MIPS_Core::clockCycles;
	return 0;
}

// finish all (or some?) DRAM commands
void DRAM::finishExecution()
{
	bufferUpdate();
	cout << "\nThe Row Buffer was updated " << rowBufferUpdates << " times. (including update on final write-back)\n";
	cout << "\nFollowing are the non-zero data values:\n";
	for (int i = 0; i < ROWS; ++i)
		for (int j = 0; j < ROWS / 4; ++j)
			if (data[i][j] != 0)
				cout << (ROWS * i + 4 * j) << '-' << (ROWS * i + 4 * j) + 3 << hex << ": " << data[i][j] << '\n'
					 << dec;
	cout << "\nTotal number of instructions executed in " << M << " cycles is: " << MIPS_Core::instructionsCount << "\nIPC = " << MIPS_Core::instructionsCount * 1.0 / M << '\n';
}

// finish the currently running DRAM instruction and set the next one
void DRAM::finishCurrDRAM()
{
	auto &Q = DRAMbuffer[currCore][currRow][currCol];
	int nextCore = currCore, nextRow = currRow, nextCol = currCol;
	QElem top = DRAMbuffer[currCore][currRow][currCol].front();
	delay = 0;
	popAndUpdate(Q, nextCore, nextRow, nextCol);

	MIPS_Core::clockCycles += top.remainingCycles;
	if (!top.id)
	{
		++rowBufferUpdates;
		buffer[currCol] = top.value;
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles);
	}
	else if (cores[top.core]->registersAddrDRAM[top.value] != make_pair(-1, -1))
	{
		cores[top.core]->registers[top.value] = buffer[currCol];
		if (cores[top.core]->registersAddrDRAM[top.value].first == top.issueCycle)
		{
			cores[top.core]->registersAddrDRAM[top.value] = {-1, -1};
			if (priority[currCore] == top.value)
				priority[currCore] = -1;
		}
		cores[top.core]->lastAddr.first = currCol * 4 + currRow * ROWS;
		cores[top.core]->lastAddr.second = buffer[currCol];
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles);
	}
	else
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles, "rejected");

	setNextDRAM(nextCore, nextRow, nextCol);
}

// set the next DRAM command to be executed (implements reordering)
void DRAM::setNextDRAM(int nextCore, int nextRow, int nextCol)
{
	if (totPending == 0)
	{
		currCore = -1;
		return;
	}
	if (priority[nextCore] != -1)
	{
		int row = cores[nextCore]->registersAddrDRAM[priority[nextCore]].second / ROWS;
		if (numProcessed == 0 || row == nextRow)
		{
			nextRow = row;
			nextCol = (cores[nextCore]->registersAddrDRAM[priority[nextCore]].second % ROWS) / 4;
		}
	}

	QElem top = DRAMbuffer[nextCore][nextRow][nextCol].front();
	if (top.id && cores[top.core]->registersAddrDRAM[top.value].first != top.issueCycle)
	{
		popAndUpdate(DRAMbuffer[nextCore][nextRow][nextCol], nextCore, nextRow, nextCol, true);
		setNextDRAM(nextCore, nextRow, nextCol);
		return;
	}

	int selectionTime = delay / 2;
	if (selectionTime != 0)
	{
		int end = MIPS_Core::clockCycles + selectionTime;
		cout << "(Memory manager) " << MIPS_Core::clockCycles << '-' << end << ": Deciding next instruction to execute" << (end > M ? " (but max clock cycles exceeded)" : "") << "\n\n";
	}
	bufferUpdate(nextCore, nextRow, nextCol);
	DRAMbuffer[currCore][currRow][currCol].front().startCycle = MIPS_Core::clockCycles + 1 + selectionTime;
	DRAMbuffer[currCore][currRow][currCol].front().remainingCycles = delay + selectionTime;
}

// pop the queue element and update the row and column if needed (returns false if DRAM empty after pop)
void DRAM::popAndUpdate(queue<QElem> &Q, int &core, int &row, int &col, bool skip)
{
	--DRAMsize, --totPending, --pendingCount[core], ++numProcessed, ++delay;
	if (MIPS_Core::clockCycles + delay / 2 > M)
	{
		--delay;
		return;
	}
	if (skip)
		printDRAMCompletion(Q.front().core, Q.front().PCaddr, MIPS_Core::clockCycles + delay / 2, MIPS_Core::clockCycles + delay / 2, "skipped");
	Q.pop();
	++MIPS_Core::instructionsCount;
	if (Q.empty())
	{
		DRAMbuffer[core][row].erase(col);
		if (DRAMbuffer[core][row].empty())
		{
			DRAMbuffer[core].erase(row);
			numProcessed = maxToProcess;
		}
		else
			col = DRAMbuffer[core][row].begin()->first;
	}
	if (totPending == 0)
	{
		numProcessed = 0;
		return;
	}
	if (numProcessed == maxToProcess)
	{
		delay += 2;
		if (MIPS_Core::clockCycles + delay / 2 > M)
			delay -= 2;
		numProcessed = 0;
		int nextCore = cores.size();
		for (int i = 1; i <= (int)cores.size(); ++i)
			if (priority[(i + core) % cores.size()] != -1)
			{
				nextCore = (i + core) % cores.size();
				break;
			}
		if (nextCore == (int)cores.size())
			for (int i = 1; i <= (int)cores.size(); ++i)
				if (pendingCount[(i + core) % cores.size()] != 0)
				{
					nextCore = (i + core) % cores.size();
					break;
				}
		core = nextCore;
		row = DRAMbuffer[core].begin()->first;
		col = DRAMbuffer[core][row].begin()->first;
	}
}

// implement buffer update
void DRAM::bufferUpdate(int core, int row, int col)
{
	if (row == -1)
	{
		delay = (currRow != -1) * row_access_delay;
		if (currRow != -1)
			++rowBufferUpdates, data[currRow] = buffer, buffer = vector<int>();
	}
	else if (currRow == -1)
		delay = row_access_delay + col_access_delay, ++rowBufferUpdates, buffer = data[row];
	else if (currRow != row)
		delay = 2 * row_access_delay + col_access_delay, ++rowBufferUpdates, data[currRow] = buffer, buffer = data[row];
	else
		delay = col_access_delay;
	currCore = core, currRow = row, currCol = col;
}

// prints the cycle info of DRAM delay
void DRAM::printDRAMCompletion(int core, int PCaddr, int begin, int end, string action)
{
	cout << "(Core " << core << ") ";
	cout << begin << '-' << end << " (DRAM call " << action << "): (PC address " << PCaddr << ") ";
	for (auto s : cores[core]->commands[PCaddr])
		cout << s << ' ';
	cout << "\n\n";
}
