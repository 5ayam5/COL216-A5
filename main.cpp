#include <bits/stdc++.h>
#include "DRAM.hpp"
#include "MIPS_Core.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		cerr << "Required arguments: file_name ROW_ACCESS_DELAY COL_ACCESS_DELAY\n";
		return 0;
	}
	ifstream file(argv[1]);
	MIPS_Core *mips;
	DRAM *dram = new DRAM();
	if (file.is_open())
		try
		{
			mips = new MIPS_Core(file, dram);
		}
		catch (exception &e)
		{
			cerr << "Required arguments: file_name ROW_ACCESS_DELAY COL_ACCESS_DELAY\n";
			return 0;
		}
	else
	{
		cerr << "File could not be opened. Terminating...\n";
		return 0;
	}

	mips->executeCommands();
	return 0;
}
