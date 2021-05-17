main:
    addi $t0, $0, 10
    sub $t1, $0, $t0
    addi $s0, $t1, 12
    add	$s0, $t1, $t0
    sw $t0, 32($zero)
    sw $t2, 0($s0)
    sw $t3, 20($s0)
    sw $t4, 40($s0)
    sw $t5, 80($s0)
    lw $t1,  8($t1)
    lw $t2, 0($s0)
    lw $t3, 20($s0)
    lw $t3, 40($s0)
    lw $t4, 40($s0)
    lw $t5,  4($s0)
    add	$t4, $t1, $s0
    j exit
exit:
