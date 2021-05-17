addi $t0, $zero, 10
addi $t1, $zero, 5
addi $s0, $zero, 20
addi $s1, $zero, 20

sw $t0, 0($s0)

lw $t1, 0($s0)
lw $t3, 4($s0)

addi $t3, $zero, 12
addi $t4, $t0, 20

sw $t3, 0($s0)
sw $t5, 100($s0)

addi $t1, $t0, 1
addi $t2, $t4, 20

