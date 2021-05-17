main:
    addi $t0, $t0, 2000
    sw $t0, 2000($zero)
    lw $s0, 2000($zero)
    sw $t1, 0($s0)
    sw $t0, 0($s0)
    j exit
exit:
