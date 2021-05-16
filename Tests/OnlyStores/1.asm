addi $t0, $0, 10
addi $t2, $0, 1000
loop:
	sw $s0, ($t2)
	addi $t0, $t0, -1
	bne $t0, $0, loop
