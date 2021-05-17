addi $t0, $zero, 10
addi $t1, $zero, 20
addi $t2, $t1, 20
addi $s0, $zero, 1020
addi $s1, $t2, 2000
lw $t2, 0($s1)

sw $t0, 0($s0)
sw $t1, 0($s1)
lw $t1, 0($s0)