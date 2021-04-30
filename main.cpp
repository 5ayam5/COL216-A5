#include <bits/stdc++.h>
#include "DRAM.hpp"
#include "MIPS_Core.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 6)
	{
		cout << "Required arguments: folder_name N M ROW_ACCESS_DELAY COL_ACCESS_DELAY\n";
		return 0;
	}
	int n = stoi(argv[2]), m = stoi(argv[3]), rowDelay = stoi(argv[4]), colDelay = stoi(argv[5]);
	if (n > MIPS_Core::MAX_CORES)
	{
		cout << "Cannot have more than " << MIPS_Core::MAX_CORES << " cores\n";
		return 0;
	}
	string folder = string(argv[1]) + '/';
	DRAM *dram = new DRAM(rowDelay, colDelay);
	MIPS_Core::dram = dram;
	for (int i = 0; i < n; ++i)
	{
		ifstream file(folder + to_string(i) + ".asm");
		if (file.is_open())
			dram->cores.push_back(new MIPS_Core(file, i));
		else
		{
			cout << "File number " << i << " could not be opened\n";
			return 0;
		}
	}

	dram->simulateExecution(m);

	return 0;
}
