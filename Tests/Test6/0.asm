addi $t0, $0, 100
sub $t1, $0, $t0
addi $s0, $t1, 12
sw $t0, 32($zero)
sw $t2, 0($s0)
lw $t1,  8($t1)
lw $t3,  4($s0)
add	$t4, $t1, $t2