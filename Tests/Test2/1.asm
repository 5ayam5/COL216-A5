main:
	addi $t0, $t0, 960
	addi $t1, $t1, 100
	sw $t0, 1024($zero)
	sw $t0, 2052($zero)
	lw $t2, 1072($zero)
	lw $t3, 1092($zero)
	lw $t1, 2180($zero)
	sw $t1, 1052($zero)	
exit:
