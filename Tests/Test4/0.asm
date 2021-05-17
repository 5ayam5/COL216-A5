addi $t0, $0, 10
addi $t1, $0, 1000
loop:
	sw $s0, ($t1)
	addi $t1, $t1, 4
	addi $t0, $t0, -1
	bne $t0, $0, loop
