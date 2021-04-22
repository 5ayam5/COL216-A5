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
}

// finish the currently running DRAM instruction and set the next one
void DRAM::finishCurrDRAM(int nextRegister)
{
	auto &Q = DRAMbuffer[currRow][currCol];
	int nextRow = currRow, nextCol = currCol;
	QElem top = DRAMbuffer[currRow][currCol].front();
	popAndUpdate(Q, nextRow, nextCol);

	MIPS_Core::clockCycles += top.remainingCycles;
	if (!top.id)
	{
		buffer[currCol] = top.value;
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles);
	}
	else if (cores[top.core]->registersAddrDRAM[top.value] != make_pair(-1, -1))
	{
		cores[top.core]->registers[top.value] = buffer[currCol];
		if (cores[top.core]->registersAddrDRAM[top.value].first == top.issueCycle)
		{
			cores[top.core]->registersAddrDRAM[top.value] = {-1, -1};
			if (nextRegister == top.value)
				nextRegister = 32;
		}
		cores[top.core]->lastAddr.first = currCol * 4 + currRow * ROWS;
		cores[top.core]->lastAddr.second = buffer[currCol];
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles);
	}
	else
		printDRAMCompletion(top.core, top.PCaddr, top.startCycle, MIPS_Core::clockCycles, "rejected");

	setNextDRAM(top.core, nextRow, nextCol, nextRegister);
}

// @TODO: implement logic for multi core
// set the next DRAM command to be executed (implements reordering)
void DRAM::setNextDRAM(int core, int nextRow, int nextCol, int nextRegister)
{
	if (DRAMbuffer.empty())
	{
		currCol = -1;
		return;
	}
	if (nextRegister != 32)
	{
		nextRow = cores[core]->registersAddrDRAM[nextRegister].second / ROWS;
		nextCol = (cores[core]->registersAddrDRAM[nextRegister].second % ROWS) / 4;
	}

	QElem top = DRAMbuffer[nextRow][nextCol].front();
	while (top.id && cores[top.core]->registersAddrDRAM[top.value].first != top.issueCycle && popAndUpdate(DRAMbuffer[nextRow][nextCol], nextRow, nextCol, true))
		top = DRAMbuffer[nextRow][nextCol].front();

	if (DRAMbuffer.empty())
	{
		currCol = -1;
		return;
	}
	bufferUpdate(nextRow, nextCol);
	DRAMbuffer[currRow][currCol].front().startCycle = cores[top.core]->clockCycles + 1;
	DRAMbuffer[currRow][currCol].front().remainingCycles = delay;
}

// @TODO: implement logic for multi core
// pop the queue element and update the row and column if needed (returns false if DRAM empty after pop)
bool DRAM::popAndUpdate(queue<QElem> &Q, int &row, int &col, bool skip)
{
	if (skip)
		printDRAMCompletion(Q.front().core, Q.front().PCaddr, cores[Q.front().core]->clockCycles, cores[Q.front().core]->clockCycles, "skipped");
	Q.pop();
	--DRAMsize;
	if (Q.empty())
	{
		DRAMbuffer[row].erase(col);
		if (DRAMbuffer[row].empty())
		{
			DRAMbuffer.erase(row);
			if (!DRAMbuffer.empty())
				row = DRAMbuffer.begin()->first;
		}
		if (DRAMbuffer.empty())
			return false;
		col = DRAMbuffer[row].begin()->first;
		return true;
	}
	return false;
}

// implement buffer update
void DRAM::bufferUpdate(int row, int col)
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
	currRow = row, currCol = col;
}

// prints the cycle info of DRAM delay
void DRAM::printDRAMCompletion(int core, int PCaddr, int begin, int end, string action)
{
	cout << "Core " << core << " -\n";
	cout << begin << '-' << end << " (DRAM call " << action << "): (PC address " << PCaddr << ") ";
	for (auto s : cores[core]->commands[PCaddr])
		cout << s << ' ';
	cout << "\n\n";
}
