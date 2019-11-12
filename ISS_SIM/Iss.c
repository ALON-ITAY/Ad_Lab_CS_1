#define MAX_NUM_OF_LINES 4096
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


int memin_to_outArr(FILE *memin_p, long int **output_arr);
void get_command_values(int *dst, int *src0, int *src1, int **vals, long int *output_arr, int pc);
void print_trace(long int *vals, long int *output_arr, FILE *trace_p, int dst, int src0, int src1);
void add(long int vals[], int dst, int src0, int src1);
void sub(long int vals[], int dst, int src0, int src1);
void and(long int vals[], int dst, int src0, int src1);
void or (long int vals[], int dst, int src0, int src1);
void xor(long int vals[], int dst, int src0, int src1);
void ld(long int *vals, int src0, int src1, long int *output_arr);
void st(long int *vals, int src0, int src1, long int *output_arr);
void jeq(long int vals[], int dst, int src0, int src1);
void jin(long int vals[], int dst, int src0, int src1);
void jlt(long int vals[], int dst, int src0, int src1);
void jle(long int vals[], int dst, int src0, int src1);
void jne(long int vals[], int dst, int src0, int src1);
void lhi(long int vals[], int dst, int src0, int src1);
void lsf(long int vals[], int dst, int src0, int src1);
void rsf(long int vals[], int dst, int src0, int src1);

void print_to_files(FILE *memout_p, long int *output_arr, int *vals);
void free_mem(long int **output_arr, long int **vals);

int main(int argc, char *argv[]) {

	FILE *memin_p = NULL, *memout_p = NULL, *trace_p = NULL; //file pointers
	long int *output_arr = (long int*) calloc(MAX_NUM_OF_LINES, sizeof(long int)); //array of memout values line by line
	long int *vals = (long int*) calloc(15, sizeof(long int));// initialze hex array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
	int dst, src0, src1, imm; //command fields
	int pc = 0;

	if (output_arr == NULL || vals == NULL) { printf("Error: failed allocating memory. \n"); exit(1); }
	assert(argc == 2);//assert 2 input file paths

	//open files
	if ((memin_p = fopen(argv[1], "r")) == NULL || (memout_p = fopen("sram_out.txt", "w")) == NULL
		 || (trace_p = fopen("trace.txt", "w")) == NULL ){	printf("Error: failed opening file. \n");}

	int LINES_IN_PROG = memin_to_outArr(memin_p, &output_arr); //copy contents of memin to memout_arr
	fprintf(trace_p, "program %s loaded, %d lines\n\n", argv[1], LINES_IN_PROG);

	while (1) {
		pc = vals[0];
		get_command_values(&dst, &src0, &src1, &vals, output_arr, pc);// get curr command feilds values.
		print_trace(vals, output_arr, trace_p, dst, src0, src1);
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
			ld(vals, src0, src1, output_arr);
		else if (vals[2] == ST)
			st(vals, src0, src1, output_arr);
		else if (vals[2] == JLT)
			jlt(vals, dst, src0, src1);
		else if (vals[2] == JLE)
			jle(vals, dst, src0, src1);
		else if (vals[2] == JEQ)
			jeq(vals, dst, src0, src1);
		else if (vals[2] == JNE)
			jne(vals, dst, src0, src1);
		else if (vals[2] == JIN)
			jin(vals, dst, src0, src1);
		else if (vals[2] == HLT)
			break;	//halt	
		vals[0]++;	//pc++; 
	}

	print_to_files(memout_p, output_arr, vals);

	//close files
	if (fclose(memin_p) == EOF || fclose(memout_p) == EOF || fclose(trace_p) == EOF) {
		printf("Error: failed closing file. \n");}
	free_mem(&output_arr, &vals);//free alocated memory
	return 0;
}


/** memin_to_outArr
 * -----------------
 * Copies content of memin text file - to the 2 dimentional output_arr, line by line.
 *
 * @param char *memin_p - A pointer to the memin.txt file.
 * @param char ***output_arr - A pointer to a 2 dimentional array to be written into.
 *
 * @return - void.
 */
int memin_to_outArr(FILE *memin_p, long int **output_arr) {
	int line = 0;

	while (line < MAX_NUM_OF_LINES && fscanf(memin_p, "%x", &((*output_arr)[line])) != EOF)
		line++;
	return line;
}

void print_trace(long int *vals, long int *output_arr, FILE *trace_p, int dst, int src0, int src1) {
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
	}
	// print trace values
	fprintf(trace_p, "--- instruction %i (%04x) @ PC %i (%04x) -----------------------------------------------------------\n",
		vals[0], vals[0], vals[0], vals[0]);
	fprintf(trace_p, "pc = %04d, inst = %08x, opcode = %01d (%s), ", vals[0], vals[1], vals[2], opcode_str);


	fprintf(trace_p, "dst = %i, src0 = %i, src1 = %i, immediate = %08x\n", vals[3], vals[4], vals[5], vals[6]);
	fprintf(trace_p, "r[0] = %08x r[1] = %08x r[2] = %08x r[3] = %08x \nr[4] = %08x r[5] = %08x r[6] = %08x r[7] = %08x \n\n",
		vals[7], vals[8], vals[9], vals[10], vals[11], vals[12], vals[13], vals[14]);

	switch (vals[2]) {
	case ADD: case SUB: case LSF: case RSF: case AND: case OR: case XOR: case LHI:
		fprintf(trace_p, ">>>> EXEC: R[%i] = %i %s %i <<<<\n\n", dst, vals[REGS_OFFSET_IN_VALS+src0], opcode_str, vals[REGS_OFFSET_IN_VALS+src1]);
		break;
	case LD:
		fprintf(trace_p, ">>>> EXEC: R[%i] = MEM[%i] = %08i <<<<\n\n", dst, vals[REGS_OFFSET_IN_VALS+src1], output_arr[vals[REGS_OFFSET_IN_VALS + src1]]);
		break;
	case ST:
		fprintf(trace_p, ">>>> EXEC: MEM[%i] = R[%i] = %08x <<<<\n\n", vals[REGS_OFFSET_IN_VALS+src1], src0, vals[REGS_OFFSET_IN_VALS+src0]);
		break;
	case JLE: case JEQ: case JNE: case JLT:
		fprintf(trace_p, ">>>> EXEC: %s %i, %i, %i <<<<\n\n", opcode_str, vals[REGS_OFFSET_IN_VALS+src0], vals[REGS_OFFSET_IN_VALS+src1], vals[0]+1);
		break;
	case JIN:
		fprintf(trace_p, ">>>> EXEC: %s %i, %i, %i <<<<\n\n", opcode_str, vals[REGS_OFFSET_IN_VALS+src0], vals[REGS_OFFSET_IN_VALS+src1], vals[IMM_OFFSET_IN_VALS]);
		break;
	case HLT:
		fprintf(trace_p, ">>>> EXEC: HALT at PC %04x<<<<\n", vals[0]);
		break;
	//default:
	}
}


/** get_command_values
 * --------------------
 * Insert values of current command feilds into variables.
 *
 * @params int *rd, *rs, *rm, *imm - Pointers to variables to be written with command feilds.
 * @param char **output_arr - 2 dimentional array with current memory image.
 * @param int pc - program counter.

 * @return - void.
 */
void get_command_values(int *dst, int *src0, int *src1, int **vals, long int *output_arr, int pc) {
	int curr_instruction;

	//convert instruction from memory to hex
	(*vals)[1] = output_arr[pc];// vals[1] is hex encoding of the current instruction.

	//extract opcode
	(*vals)[2] = (*vals)[1] & 0x3E000000;
	(*vals)[2] = (*vals)[2] >> 0x19;
	//extract dst
	*dst = (*vals)[1] & 0x01c00000;
	*dst = *dst >> 0x16;
	(*vals)[3] = *dst;
	//extract src0
	*src0 = (*vals)[1] & 0x00380000;
	*src0 = *src0 >> 0x13;
	(*vals)[4] = *src0;
	//extract src1
	*src1 = (*vals)[1] & 0x00070000;
	*src1 = *src1 >> 0x10;
	(*vals)[5] = *src1;
	//extract immediate
	(*vals)[REGS_OFFSET_IN_VALS+1] = (*vals)[1] & 0x0000ffff; //reg1 is immediate
	(*vals)[6] = (*vals)[REGS_OFFSET_IN_VALS + 1]; //copy value of immidiate to it's position in vals array

	//adjust register offset in case of using immediate
	if (*src0 == 1) { *src0 = -1; } //correct the register offset to be the immidiate offset in "vals" array
	if (*src1 == 1) { *src1 = -1; } //correct the register offset to be the immidiate offset in "vals" array
}

/** add
 * -----
 * Computes addition of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs, rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the addition computation.
 *
 * @return - void.
 */
void add(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] + vals[src1 + REGS_OFFSET_IN_VALS];
}


/** sub
 * -----
 * Computes subtruction of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void sub(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] - vals[src1 + REGS_OFFSET_IN_VALS];
}
/** LSF
 * -----
 * Computes left shift of src0 of src1 places.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void lsf(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] << vals[src1 + REGS_OFFSET_IN_VALS];
}
/** RSF
 * -----
 * Computes right shift of src1 of src0 places.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void rsf(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] >> vals[src1 + REGS_OFFSET_IN_VALS];
}
/** and
 * -----
 * Computes bitwise and of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void and(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] & vals[src1 + REGS_OFFSET_IN_VALS];
}
/** or
 * -----
 * Computes bitwise or of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void or(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] | vals[src1 + REGS_OFFSET_IN_VALS];
}
/** xor
 * -----
 * Computes bitwise xor of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void xor(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS] ^ vals[src1 + REGS_OFFSET_IN_VALS];
}
/** LHI
 * -----
 * Computes bitwise xor of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void lhi(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[dst + REGS_OFFSET_IN_VALS] = (vals[dst + REGS_OFFSET_IN_VALS]& 0x0000ffff)|((vals[src0 + REGS_OFFSET_IN_VALS] | vals[src1 + REGS_OFFSET_IN_VALS])<<0x10);
}
/** LD
 * -----
 * Computes bitwise xor of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void ld(long int *vals, int src0, int src1, long int *output_arr) {
	if (src0 == 0) { //dont change the zero register
		return;
	}

	vals[src0 + REGS_OFFSET_IN_VALS] = output_arr[src1 + REGS_OFFSET_IN_VALS];
}

/** ST
 * -----
 * Computes bitwise xor of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void st(long int vals[], int src0, int src1, long int *output_arr) {
	output_arr[src1 + REGS_OFFSET_IN_VALS] = vals[src0 + REGS_OFFSET_IN_VALS];	
}
/** JLT
 * -----
 * Computes left shift of src0 of src1 places.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void jlt(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	if(vals[src0 + REGS_OFFSET_IN_VALS] < vals[src1 + REGS_OFFSET_IN_VALS]){
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0];
		vals[0] = (vals[REGS_OFFSET_IN_VALS] & 65535)-1;
	}
}
/** JLE
 * -----
 * Computes right shift of src1 of src0 places.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void jle(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	if(vals[src0 + REGS_OFFSET_IN_VALS] <= vals[src1 + REGS_OFFSET_IN_VALS]){
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0];
		vals[0] = (vals[REGS_OFFSET_IN_VALS] & 65535)-1;
	}
}
/** JEQ
 * -----
 * Computes bitwise and of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void jeq(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	if(vals[src0 + REGS_OFFSET_IN_VALS] == vals[src1 + REGS_OFFSET_IN_VALS]){
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0];
		vals[0] = (vals[REGS_OFFSET_IN_VALS] & 65535)-1;
	}
}
/** JNE
 * -----
 * Computes bitwise and of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void jne(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	if(vals[src0 + REGS_OFFSET_IN_VALS] != vals[src1 + REGS_OFFSET_IN_VALS]){
		vals[7 + REGS_OFFSET_IN_VALS] = vals[0];
		vals[0] = (vals[REGS_OFFSET_IN_VALS] & 65535)-1;
	}
}
/** JIN
 * -----
 * Computes bitwise and of two integers and a constant.
 *
 * @param int *vals - Array containing: pc, current command coding, and register values.
 * @params int rd, rs,rt - variables indicating registers index (index in vals array + 2).
 * @param int imm - constant used in the subtruction computation.
 *
 * @return - void.
 */
void jin(long int vals[], int dst, int src0, int src1) {
	if (dst == 0 || dst == 1) { //dont change the zero register
		return;
	}
	vals[7 + REGS_OFFSET_IN_VALS] = vals[0];
	vals[0] = vals[src0 + REGS_OFFSET_IN_VALS];
}

/** print_to_files
 * ----------------
 * Print data to files at end of simolator run.
 *
 * @params FILE *count_p, *regout_p, *memout_p - Pointers to files to be written to.
 * @param char **output_arr - 2 dimentional array with final memory image.
 * @param int command_cnt - number of command executed.
 * @param int *vals - Array containing: pc, current command coding, and register values.

 * @return - void.
 */
void print_to_files(FILE *memout_p, long int *output_arr, int *vals) {
	int i = 0, last_ind = 0;

	// find the last memory index
	for (i = MAX_NUM_OF_LINES - 1; i >= 0; i--) {
		if (output_arr[i] == 0) {
			last_ind = i;
			break;
		}
	}
	// print memout file. 
	for (i = 0; i <= last_ind; i++) {
		if (fprintf(memout_p, "%08x\n", output_arr[i]) < 0) {
			printf("Error: failed writing to file 'memout.txt'. \n");
		}
	}
	return;
}


/** free_mem
 * ----------
 * Frees dynamic memory previously allocated for the simulator run.
 *
 * @param char ***output_arr - A pointer to a 2 dimentional array to be freed.
 *
 * @return - void.
 */
void free_mem(long int **output_arr, long int **vals) {
	free(*output_arr);
	free(*vals);
	return;
}
