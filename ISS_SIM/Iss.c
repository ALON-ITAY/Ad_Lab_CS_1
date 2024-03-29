#define _CRT_SECURE_NO_WARNINGS 1 
#define _WINSOCK_DEPRECATED_NO_WARNINGS 1 

#define MAX_NUM_OF_LINES 65536
#define COMMAND_LEN 8
#define REGS_OFFSET_IN_VALS 7
#define IMM_OFFSET_IN_VALS 6
#define ADD 0
#define SUB 1
#define LSF 2
#define RSF 3
#define AND 4
#define OR  5
#define XOR 6
#define LHI 7
#define LD 8
#define ST 9
#define JLT 16
#define JLE 17
#define JEQ 18
#define JNE 19
#define JIN 20
#define HLT 24


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(int argc, char * argv[]);


int memin_to_outArr(FILE *memin_p, long int *output_arr);
void get_command_values(int *dst, int *src0, int *src1, long int *vals, long int *output_arr, int pc);
void print_trace(long int *vals, long int *output_arr, FILE *trace_p, int dst, int src0, int src1, int inst_cnt);
void print_jump_exec_to_trace(FILE* trace_p, long int *vals, int src0, int src1);
void add(long int vals[], int dst, int src0, int src1);
void sub(long int vals[], int dst, int src0, int src1);
void and(long int vals[], int dst, int src0, int src1);
void or (long int vals[], int dst, int src0, int src1);
void xor(long int vals[], int dst, int src0, int src1);
void ld(long int vals[], int dst, int src1, long int *output_arr);
void st(long int vals[], int src0, int src1, long int *output_arr);
void jeq(long int vals[], int src0, int src1);
void jin(long int vals[], int src0);
void jlt(long int vals[], int src0, int src1);
void jle(long int vals[], int src0, int src1);
void jne(long int vals[], int src0, int src1);
void lhi(long int vals[], int dst, int src0, int src1);
void lsf(long int vals[], int dst, int src0, int src1);
void rsf(long int vals[], int dst, int src0, int src1);
void print_to_files(FILE *memout_p, FILE *trace_p, long int *output_arr, long int *vals, int inst_cnt);
void free_mem(long int *output_arr, long int *vals);

int main(int argc, char *argv[]) {

	FILE *memin_p = NULL, *memout_p = NULL, *trace_p = NULL; //file pointers
	long int *output_arr = (long int*) calloc(MAX_NUM_OF_LINES, sizeof(long int)); //array of memout values line by line
	long int *vals = (long int*) calloc(15, sizeof(long int));// initialze hex array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
	int dst, src0, src1; //command fields
	int pc = 0, inst_cnt = 0;

	if (output_arr == NULL || vals == NULL) { printf("Error: failed allocating memory. \n"); exit(1); }
	assert(argc == 2);//assert 2 input file paths

	//open files
	if ((memin_p = fopen(argv[1], "r")) == NULL || (memout_p = fopen("sram_out.txt", "w")) == NULL
		|| (trace_p = fopen("trace.txt", "w")) == NULL) {
		printf("Error: failed opening file. \n"); exit(1);
	}

	int LINES_IN_PROG = memin_to_outArr(memin_p, output_arr); //copy contents of memin to memout_arr
	fprintf(trace_p, "program %s loaded, %d lines\n\n", argv[1], LINES_IN_PROG);

	while (1) {
		pc = vals[0];
		get_command_values(&dst, &src0, &src1, vals, output_arr, pc);// get curr command feilds values.
		print_trace(vals, output_arr, trace_p, dst, src0, src1, inst_cnt);
		//execute command
		if (vals[2] == ADD)
			add(vals, dst, src0, src1);
		else if (vals[2] == SUB)
			sub(vals, dst, src0, src1);
		else if (vals[2] == LSF)
			lsf (vals, dst, src0, src1);
		else if (vals[2] == RSF)
			rsf (vals, dst, src0, src1);
		else if (vals[2] == AND)
			and(vals, dst, src0, src1);
		else if (vals[2] == OR)
			or(vals, dst, src0, src1);
		else if (vals[2] == XOR)
			xor(vals, dst, src0, src1);
		else if (vals[2] ==LHI)
			lhi(vals, dst, src0, src1);
		else if (vals[2] == LD)
			ld(vals, dst, src1, output_arr);
		else if (vals[2] == ST)
			st(vals, src0, src1, output_arr);
		else if (vals[2] == JLT)
			jlt(vals, src0, src1);
		else if (vals[2] == JLE)
			jle(vals, src0, src1);
		else if (vals[2] == JEQ)
			jeq(vals, src0, src1);
		else if (vals[2] == JNE)
			jne(vals, src0, src1);
		else if (vals[2] == JIN)
			jin(vals, src0);
		else if (vals[2] == HLT){
			inst_cnt++;
			break;	//halt	
		}
		vals[0]++;	//pc++;
		print_jump_exec_to_trace(trace_p, vals, src0, src1);
		inst_cnt++;
	}

	print_to_files(memout_p, trace_p ,output_arr, vals, inst_cnt);

	//close files
	if (fclose(memin_p) == EOF || fclose(memout_p) == EOF || fclose(trace_p) == EOF) {
		printf("Error: failed closing file. \n");}
	free_mem(output_arr, vals);//free alocated memory
	return 0;
}


/** memin_to_outArr
 * -----------------
 * Copies content of memin text file - to the 2 dimentional output_arr, line by line.
 *
 * @param char *memin_p - A pointer to the memin.txt file.
 * @param long int **output_arr - A pointer to the output data array to be written into.
 *
 * @return - void.
 */
int memin_to_outArr(FILE *memin_p, long int *output_arr) {
	int line = 0;

	while (line < MAX_NUM_OF_LINES && fscanf(memin_p, "%08x",(unsigned int*) &(output_arr[line])) != EOF)
		line++;
	return line;
}


/** print_trace
 * --------------
 * Prints program trace.
 *
 * @param long int *vals -  array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
 * @param long int *output_arr - A pointer to the output data array to be written into.
 * @param FILE *trace_p - pointer to trace output file.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 * @param int inst_cnt - instruction count.
 *
 * @return - void.
 */
void print_trace(long int *vals, long int *output_arr, FILE *trace_p, int dst, int src0, int src1, int inst_cnt) {
	char* opcode_str;

	switch (vals[2]) {
	case ADD: opcode_str = "ADD";
		break;
	case SUB: opcode_str = "SUB";
		break;
	case LSF: opcode_str = "LSF";
		break;
	case RSF: opcode_str = "RSF";
		break;
	case AND: opcode_str = "AND";
		break;
	case OR: opcode_str = "OR";
		break;
	case XOR: opcode_str = "XOR";
		break;
	case LHI: opcode_str = "LHI";
		break;
	case LD: opcode_str = "LD";
		break;
	case ST: opcode_str = "ST";
		break;
	case JLT: opcode_str = "JLT";
		break;
	case JLE: opcode_str = "JLE";
		break;
	case JEQ: opcode_str = "JEQ";
		break;
	case JNE: opcode_str = "JNE";
		break;
	case JIN: opcode_str = "JIN";
		break;
	case HLT: opcode_str = "HLT";
		break;
	default: opcode_str = "";
	}
	// print trace values
	fprintf(trace_p, "--- instruction %i (%04x) @ PC %ld (%04lx) -----------------------------------------------------------\n",
		inst_cnt, inst_cnt, vals[0], vals[0]);
	fprintf(trace_p, "pc = %04ld, inst = %08lx, opcode = %ld (%s), ", vals[0], vals[1], vals[2], opcode_str);


	fprintf(trace_p, "dst = %ld, src0 = %ld, src1 = %ld, immediate = %08lx\n", vals[3], vals[4], vals[5], vals[6]);
	fprintf(trace_p, "r[0] = %08lx r[1] = %08lx r[2] = %08lx r[3] = %08lx \nr[4] = %08lx r[5] = %08lx r[6] = %08lx r[7] = %08lx \n\n",
		vals[7], vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14]);

	switch (vals[2]) {
	case ADD: case SUB: case LSF: case RSF: case AND: case OR: case XOR: case LHI:
		fprintf(trace_p, ">>>> EXEC: R[%d] = %ld %s %ld <<<<\n\n", dst, vals[REGS_OFFSET_IN_VALS + src0], opcode_str, vals[REGS_OFFSET_IN_VALS + src1]);
		break;
	case LD:
		fprintf(trace_p, ">>>> EXEC: R[%d] = MEM[%ld] = %08lx <<<<\n\n", dst, vals[REGS_OFFSET_IN_VALS + src1], output_arr[vals[REGS_OFFSET_IN_VALS + src1]]);
		break;
	case ST:
		fprintf(trace_p, ">>>> EXEC: MEM[%ld] = R[%d] = %08lx <<<<\n\n", vals[REGS_OFFSET_IN_VALS + src1], src0, vals[REGS_OFFSET_IN_VALS + src0]);
		break;
	case HLT:
		fprintf(trace_p, ">>>> EXEC: HALT at PC %04lx<<<<\n", vals[0]);
		break;
	}
}


/** print_jump_exec_to_trace
 * --------------------------
 * Prints program "EXEC" trace for branch commands.
 *
 * @param FILE *trace_p - pointer to trace output file.
 * @param long int *vals -  array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
 * @param long int *output_arr - A pointer to the output data array to be written into.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void print_jump_exec_to_trace(FILE* trace_p, long int *vals, int src0, int src1) {

	char* opcode_str;

	switch (vals[2]) {
	case JLT: opcode_str = "JLT";
		break;
	case JLE: opcode_str = "JLE";
		break;
	case JEQ: opcode_str = "JEQ";
		break;
	case JNE: opcode_str = "JNE";
		break;
	case JIN: opcode_str = "JIN";
		break;
	default: opcode_str = "";
	}

	switch (vals[2]) {
	case JLE: case JEQ: case JNE: case JLT: case JIN:
		fprintf(trace_p, ">>>> EXEC: %s %ld, %ld, %ld <<<<\n\n", opcode_str, vals[REGS_OFFSET_IN_VALS + src0], vals[REGS_OFFSET_IN_VALS + src1], vals[0]);
		break;
	}
}


/** get_command_values
 * --------------------
 * Insert values of current command fields into variables.
 *
 * @params int *dst, *src0, *src1 - Pointers to variables to be written with command feilds.
 * @param long int *vals -  array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
 * @param long int *output_arr - A pointer to the output data array to be written into.
 * @param int pc - program counter.

 * @return - void.
 */
void get_command_values(int *dst, int *src0, int *src1, long int *vals, long int *output_arr, int pc) {

	//convert instruction from memory to hex
	vals[1] = output_arr[pc];// vals[1] is hex encoding of the current instruction.

	//extract opcode
	vals[2] = vals[1] & 0x3E000000;
	vals[2] = vals[2] >> 0x19;
	//extract dst
	*dst = vals[1] & 0x01c00000;
	*dst = *dst >> 0x16;
	vals[3] = *dst;
	//extract src0
	*src0 = vals[1] & 0x00380000;
	*src0 = *src0 >> 0x13;
	vals[4] = *src0;
	//extract src1
	*src1 = vals[1] & 0x00070000;
	*src1 = *src1 >> 0x10;
	vals[5] = *src1;
	//extract immediate
	vals[REGS_OFFSET_IN_VALS+1] = vals[1] & 0x0000ffff; //reg1 is immediate
	vals[REGS_OFFSET_IN_VALS + 1] = (vals[REGS_OFFSET_IN_VALS + 1] << 16) >> 16;
	vals[IMM_OFFSET_IN_VALS] = vals[REGS_OFFSET_IN_VALS + 1]; //copy value of immidiate to it's position in vals array

	//adjust register offset in case of using immediate
	if (*src0 == 1) { *src0 = -1; } //correct the register offset to be the immidiate offset in "vals" array
	if (*src1 == 1) { *src1 = -1; } //correct the register offset to be the immidiate offset in "vals" array
}

/** add
 * -----
 * Computes addition of two integers and a constant.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void add(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] + vals[src1 + REGS_OFFSET_IN_VALS]; /*R[dst]=R[src0] + R[src1]*/
}


/** sub
 * -----
 * Computes subtruction of two integers and a constant.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void sub(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] - vals[src1 + REGS_OFFSET_IN_VALS]; /*R[dst]=R[src0] - R[src1]*/
}
/** LSF
 * -----
 * Computes left shift of src0 of src1 places.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void lsf(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] << vals[src1 + REGS_OFFSET_IN_VALS];/*R[dst]=lsf(R[src0],R[src1])*/
}
/** RSF
 * -----
 * Computes right shift of src1 of src0 places.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void rsf(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] >> vals[src1 + REGS_OFFSET_IN_VALS]; /*R[dst]=rsf(R[src0],R[src1])*/
}
/** and
 * -----
 * Computes bitwise and of two integers and a constant.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void and(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] & vals[src1 + REGS_OFFSET_IN_VALS]; /*R[dst]=R[src0] AND R[src1]*/
}
/** or
 * -----
 * Computes bitwise or of two integers and a constant.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void or(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] | vals[src1 + REGS_OFFSET_IN_VALS]; /*R[dst]=R[src0] OR R[src1]*/
}
/** xor
 * -----
 * Computes bitwise xor of two integers and a constant.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void xor(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] ^ vals[src1 + REGS_OFFSET_IN_VALS];/*R[dst]=R[src0] XOR R[src1]*/
}
/** LHI
 * -----
 * Loads the 16 lowest bits of the immediate into the 16 highst bits of the dst register.
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void lhi(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = (vals[dst + REGS_OFFSET_IN_VALS]& 0x0000ffff)|((vals[src0 + REGS_OFFSET_IN_VALS] | vals[src1 + REGS_OFFSET_IN_VALS])<<0x10); /*R[dst][31:16]=imm[31:16]*/
}
/** LD
 * -----
 * Loads memory content at address specified by R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 * @param long int *output_arr - Array containing the memory of the program that we will output.
 *
 * @return - void.
 */
void ld(long int vals[], int dst, int src1, long int *output_arr) {
	if (dst == 0) { //dont change the zero register
		return;
	}

	vals[dst + REGS_OFFSET_IN_VALS] = output_arr[vals[src1 + REGS_OFFSET_IN_VALS]];/*R[dst]=MEM[R[src1]]*/
}

/** ST
 * -----
 * Writes R[src0] to memory at address R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 * @param long int *output_arr - Array containing the memory of the program that we will output.
 *
 * @return - void.
 */
void st(long int vals[], int src0, int src1, long int *output_arr) {
	output_arr[vals[src1 + REGS_OFFSET_IN_VALS]] = vals[src0 + REGS_OFFSET_IN_VALS];/*MEM[R[src1]]=R[src0]*/	
}
/** JLT
 * -----
 * jumps to immediate[15:0] if R[src0]<R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void jlt(long int vals[], int src0, int src1) {

	if(vals[src0 + REGS_OFFSET_IN_VALS] < vals[src1 + REGS_OFFSET_IN_VALS]){/*if r[src0]<R[src1]*/
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0]; /*store the current pc in R[7]*/
		vals[0] = (vals[IMM_OFFSET_IN_VALS] & 65535)-1; /*pc=imm[15:0]-1 becuase we later increment it so we subtract 1
		to jump to the right value of pc at the end of the execution of the comand*/
	}
}
/** JLE
 * -----
 * jumps to immediate[15:0] if R[src0]<=R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void jle(long int vals[], int src0, int src1) {

	if(vals[src0 + REGS_OFFSET_IN_VALS] <= vals[src1 + REGS_OFFSET_IN_VALS]){/*if r[src0]<=R[src1]*/
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0]; /*store the current pc in R[7]*/
		vals[0] = (vals[IMM_OFFSET_IN_VALS] & 65535)-1; /*pc=imm[15:0]-1 becuase we later increment it so we subtract 1
		to jump to the right value of pc at the end of the execution of the comand*/
	}
}
/** JEQ
 * -----
 * jumps to immediate[15:0] if R[src0]==R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void jeq(long int vals[], int src0, int src1) {

	if(vals[src0 + REGS_OFFSET_IN_VALS] == vals[src1 + REGS_OFFSET_IN_VALS]){/*if r[src0]==R[src1]*/
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0]; /*store the current pc in R[7]*/
		vals[0] = (vals[IMM_OFFSET_IN_VALS] & 65535)-1; /*pc=imm[15:0]-1 becuase we later increment it so we subtract 1
		to jump to the right value of pc at the end of the execution of the comand*/
	}
}
/** JNE
 * -----
 * jumps to immediate[15:0] if R[src0]!=R[src1].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void jne(long int vals[], int src0, int src1) {

	if(vals[src0 + REGS_OFFSET_IN_VALS] != vals[src1 + REGS_OFFSET_IN_VALS]){/*if r[src0]!=R[src1]*/
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0]; /*store the current pc in R[7]*/
		vals[0] = (vals[IMM_OFFSET_IN_VALS] & 65535)-1; /*pc=imm[15:0]-1 becuase we later increment it so we subtract 1
		to jump to the right value of pc at the end of the execution of the comand*/
	}
}
/** JIN
 * -----
 * jumps to immediate[15:0].
 *
 * @param long int vals[] - Array containing: pc, current command coding, and register values.
 * @params int dst,src0,src1 - variables indicating registers index (index in vals array + REGS_OFFSET_IN_VALS).
 *
 * @return - void.
 */
void jin(long int vals[], int src0) {

	vals[7 + REGS_OFFSET_IN_VALS] = vals[0]; /*store the current pc in R[src7]*/
	vals[0] = vals[src0 + REGS_OFFSET_IN_VALS]-1; /*pc=R[src0]*/
}

/** print_to_files
 * ----------------
 * Print data to files at end of simulator run.
 *
 * @params FILE *memout_p, *trace_p - Pointers to files to be written into.
 * @param long int *output_arr - A pointer to the output data array to be written into.
 * @param long int *vals -  array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
 * @param int inst_cnt - total number of commands executed.

 * @return - void.
 */
void print_to_files(FILE *memout_p, FILE *trace_p, long int *output_arr, long int *vals, int inst_cnt) {
	int i = 0, max_mem_line = MAX_NUM_OF_LINES;

	fprintf(trace_p, "sim finished at pc %ld, %d instructions", vals[0], inst_cnt);
	
	//find max_mem_line
	for (i = MAX_NUM_OF_LINES-1; i>=0; i--) {
		if (output_arr[i] == 0) {
			max_mem_line;
		}
	}

	// print memout file. 
	for (i = 0; i < max_mem_line; i++) {
		if (fprintf(memout_p, "%08lx\n", output_arr[i]) < 0) {
			printf("Error: failed writing to file 'memout.txt'. \n");
		}
	}
	return;
}


/** free_mem
 * ----------
 * Frees dynamic memory previously allocated for the simulator run.
 *
 * @param long int *output_arr - A pointer to the output data array to be written into.
 * @param long int *vals -  array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
 *
 * @return - void.
 */
void free_mem(long int *output_arr, long int *vals) {
	free(output_arr);
	free(vals);
	return;
}
