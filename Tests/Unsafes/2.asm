addi $t0, $0, 100
sw $t0, 1000
addi $t2, $0, 20
addi $t0, $0, 1
loop:
	addi $t0, $t0, 1
	addi $t2, $t2, -1
	bne $t2, $0, loop
lw $t0, 1000
addi $t2, $t0, -50
