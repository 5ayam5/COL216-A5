from sys import argv

with open(argv[1]) as fin, open("summary", "a+") as fout:
	lines = fin.readlines()
	fout.writelines(lines[-3:])
