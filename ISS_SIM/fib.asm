/* Init fisrt fib variables */
asm_cmd(ADD, 2, 1, 0, 1);// 0: R2 = 1
asm_cmd(ADD, 3, 1, 0, 1); // 1: R3 = 1
asm_cmd(ADD, 6, 1, 0, 1002); // 2: R6 = 1002
asm_cmd(ST,  0, 2, 1, 1000); // 3: MEM[1000] = R2
asm_cmd(ST,  0, 3, 1, 1001); // 4: MEM[1001] = R3
/* Init loop counter (38 times) */
asm_cmd(ADD, 4, 1, 0, 37); // 5: R4 = 37
/* Fib loop */
asm_cmd(JEQ, 0, 4, 0, 14); // 6: if(R4 == 0) jump to 14; else continue
asm_cmd(ADD,  5, 2, 3, 0); // 7: R5 = R2 + R3
asm_cmd(ST,  0, 5, 6, 0); // 8: MEM[R6] = R5 (Store result in memory)
asm_cmd(ADD, 2, 3, 0, 0); // 9: R2 = R3
asm_cmd(ADD, 3, 5, 0, 0); // 10: R3 = R5
asm_cmd(SUB, 4, 4, 1, 1); // 11: R4 = R4 - 1
asm_cmd(ADD, 6, 6, 1, 1); // 12: R6 = R6 + 1
asm_cmd(JEQ, 0, 0, 0, 6); // 13: jump to 6

/* Halt */
asm_cmd(HLT, 0, 0, 0, 0); // 14: return

