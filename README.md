# COL216-A5 #
_Sayam Sethi        2019CS10399_\
_Mallika Prabhakar  2019CS50440_

## Contents: ##

### Files: ###
1. Assignment-5.pdf
2. DRAM.cpp
3. DRAM.hpp
4. main.cpp
5. main.tex
6. makefile
7. MIPS_Core.cpp
8. MIPS_Core.hpp
9. overview.py
10. run.sh

### Test cases: ###
1. **Test1 -** Initial test case to test multi-core functionality
2. **Test2 -** Test case to test execution when row changes are needed (no dependent instructions)
3. **Test3 -** Tests execution when instructions are skipped + stopping a single core on error
4. **Test4 -** Contains primarily store expressions in the files 
5. **Test5 -** Each core respectively: dependent loads, no DRAM, forwarding (reduction of cycles from about 450 cycles to 271 cycles compared to Assignment 4 implementation) 
6. **Test6 -** Unsafe instructions in all cores (forwarding happens in some of the cores)
7. **Test7 -** Collection of files provided in Assignment 4 demonstration to be run in a parallel manner
8. **Test8 -** Random test case 1 (random)
9. **Test9 -** Random test case 2 (negative address)
10. **Test10 -** Random test case 3 (normal files and erroneous files run in parallel)
