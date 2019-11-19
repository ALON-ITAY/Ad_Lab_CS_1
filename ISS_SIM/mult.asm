/*Based on shift and add multiplication algorithm */
/* Get values from memory */
asm_cmd(LD,  3, 0, 1, 1000); // 0: R[3] = MEM[1000]
asm_cmd(LD,  4, 0, 1, 1001); // 1: R[4] = MEM[1001]
/* Init variables */
asm_cmd(ADD, 6, 0, 0, 0);	 // 2: R[6] = 0 (result)
asm_cmd(ADD, 2, 0, 1, 1); 	 // 3: R[2] = 1 (counter)
/*loop while count > 0*/
asm_cmd(AND, 5, 2, 4, 0); 	 // 4: R[5] = R[4] AND R[2] (R[2]'s current bit)
asm_cmd(JLE, 0, 5, 0, 7);	 // 5: if R[5] <= 0 jump to line 7
asm_cmd(ADD, 6, 6, 3, 0);	 // 6: R[6] = R[6] + R[3] (add current argument to the sum) 
asm_cmd(LSF, 3, 3, 1, 1);	 // 7: R[3] = LSF(R[3]) (shift argument)
asm_cmd(LSF, 2, 2, 1, 1);	 // 8: R[2] = LSF(R[2]) (shift counter)
asm_cmd(JLT, 0, 0, 2, 4);	 // 9: if 0 < R[2] jump to line 4
/* Store value in MEM and end program */
asm_cmd(ST,  0, 6, 1, 1002); // 10: MEM[1002] = R[6]
asm_cmd(HLT, 0, 0, 0, 0); 	 // 11: Return

