#define MAX_NUM_OF_LINES 4096
#define COMMAND_LEN 8
#define REGS_OFFSET_IN_VALS 7
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

void alloc_mem(char ***output_arr);
void memin_to_outArr(FILE *memin_p, char ***output_arr);
void get_command_values(int *dst, int *src0, int *src1, int *imm, char **output_arr, int pc);
void add(long int vals[], int dst, int src0, int src1);
void sub(long int vals[], int dst, int src0, int src1);
void print_to_files(FILE *memout_p, char **output_arr, long int *vals);
void free_mem(char ***output_arr);

int main(int argc, char *argv[]) {

	FILE *memin_p = NULL, *memout_p = NULL, *trace_p = NULL; //file pointers
	char **output_arr; //array of memout values line by line
	long int vals[] = { 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000,	0x00000000 ,0x00000000, 0x00000000,	0x00000000,	0x00000000,
		0x00000000}; // initialze hex array for values of: pc, instruction, opcode, dst, src0, src1, immediate, registers.
	long int dst, src0, src1, imm; //command feilds
	int i = 0, pc = 0;


	assert(argc == 2);//assert 2 input file paths

	//open files
	if ((memin_p = fopen(argv[1], "r")) == NULL || (memout_p = fopen("sram_out.txt", "w")) == NULL
		 || (trace_p = fopen("trace.txt", "w")) == NULL ){	printf("Error: failed opening file. \n");}

	alloc_mem(&output_arr); //alocate space for output array
	memin_to_outArr(memin_p, &output_arr); //copy contents of memin to memout_arr

	while (1) {
		pc = vals[0];
		vals[1] = (long int)strtoul(output_arr[pc], NULL, 16);// vals[1] is hex encoding of the current command.
		if (fprintf(trace_p, "%08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X\n",
			vals[0], vals[1], vals[2], vals[3], vals[4], vals[5], vals[6], vals[7], vals[8], vals[9],
			vals[10], vals[11], vals[12], vals[13],	vals[14]) < 0) {	// print trace values
			printf("Error: failed writing to file 'trace.txt'. \n");
		}
		get_command_values(&dst, &src0, &src1, &imm, output_arr, pc);// get curr command feilds values.
		vals[6] = imm; //copy value of immidiate to vals array (to the position of reg1)

		
		//exectute command
		if (vals[1] == ADD)
			add(vals, dst, src0, src1);/*
		else if (vals[1] == SUB)
			sub(vals, rd, rs, rt, imm);
		else if (vals[1] == LSF)
			and (vals, rd, rs, rt, imm);
		else if (vals[1] == RSF)
			or (vals, rd, rs, rt, imm);
		else if (vals[1] == AND)
			sll(vals, rd, rs, rt, imm);
		else if (vals[1] == OR)
			sra(vals, rd, rs, rt, imm);
		else if (vals[1] == XOR)
			mac(vals, rd, rs, rt, rm, imm);
		else if (vals[1] ==LHI)
			branch(rs, rt, rm, imm, &memout_p, vals);
		//8,9,A(10) are reserved commands
		else if (vals[1] == LD)
			jal(imm, &memout_p, vals);
		else if (vals[1] == ST)
			lw(rd, rs, imm, &output_arr, vals);
		else if (vals[1] == JLT)
			sw(rd, rs, imm, &output_arr, vals);
		else if (vals[1] == JLE)
			jr(rd, vals);
		else if (vals[1] == JNE)
			jr(rd, vals);
		else if (vals[1] == JIN)
			jr(rd, vals);
		else if (vals[1] == HLT)
			break;	//halt	
		vals[0]++;	//pc++; */
	}

	print_to_files(memout_p, output_arr, vals);

	//close files
	if (fclose(memin_p) == EOF || fclose(memout_p) == EOF || fclose(trace_p) == EOF) {
		printf("Error: failed opening file. \n");}
	free_mem(&output_arr);//free alocated memory
	return 0;
}


/** alloc_mem
 * -----------
 * Allocates dynamic memory for the simulator run.
 * Initialzes values in allocated space.
 *
 * @param char ***output_arr - A pointer to a 2 dimentional array which will contain output memory state line by line.
 *
 * @return - void.
 */
void alloc_mem(char ***output_arr) {
	int i;

	if (((*output_arr) = (char**)malloc(MAX_NUM_OF_LINES * sizeof(char*))) == NULL) {
		printf("Error: failed allocationg memory. \n");
		exit(1);
	}
	for (i = 0; i < MAX_NUM_OF_LINES; i++) {
		if (((*output_arr)[i] = (char*)malloc((COMMAND_LEN + 2) * sizeof(char))) == NULL) {
			printf("Error: failed allocationg memory. \n");
			exit(1);
		}
	}

	/*initialize output_arr values*/
	for (i = 0; i < MAX_NUM_OF_LINES; i++) {
		memset((*output_arr)[i], '0', COMMAND_LEN);
		(*output_arr)[i][COMMAND_LEN] = '\n';
		(*output_arr)[i][COMMAND_LEN + 1] = '\0';
	}
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
void memin_to_outArr(FILE *memin_p, char ***output_arr) {
	int i = 0;
	while (fgets((*output_arr)[i], COMMAND_LEN + 2, memin_p) != NULL) {
		i++;
	}
	return;
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
void get_command_values(int *dst, int *src0, int *src1, int *imm, char **output_arr, int pc) {
	char dst_str[] = "0", src0_str[] = "0", src1_str[] = "0", imm_str[] = "000";

	dst_str[0] = output_arr[pc][1]; // copy dst part from command string
	src0_str[0] = output_arr[pc][2]; // copy src0 part from command string
	src1_str[0] = output_arr[pc][3]; // copy src1 part from command string
	strncpy(imm_str, output_arr[pc] + 5, 3); // copy imm part from command string
	*dst = (long int)strtoul(dst_str, NULL, 16);
	*src0 = (long int)strtoul(src0_str, NULL, 16);
	*src1 = (long int)strtoul(src1_str, NULL, 16);
	*imm = (long int)strtoul(imm_str, NULL, 16);

	if (*src0 == 1) { *src0 = -1; } //correct the register offset to be the immidiate offset in "vals" array
	if (*src1 == 1) { *src1 = -1; } //correct the register offset to be the immidiate offset in "vals" array
}


/** add
 * -----
 * Computes addition of two integers and a constant.
 *
 * @param long int *vals - Array containing: pc, current command coding, and register values.
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
 * @param long int *vals - Array containing: pc, current command coding, and register values.
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



/** print_to_files
 * ----------------
 * Print data to files at end of simolator run.
 *
 * @params FILE *count_p, *regout_p, *memout_p - Pointers to files to be written to.
 * @param char **output_arr - 2 dimentional array with final memory image.
 * @param int command_cnt - number of command executed.
 * @param long int *vals - Array containing: pc, current command coding, and register values.

 * @return - void.
 */
void print_to_files(FILE *memout_p, char **output_arr, long int *vals) {
	int i = 0, last_ind = 0;
	size_t vals_len = sizeof(vals) / sizeof(vals[0]);

	// find the last memory index
	for (i = MAX_NUM_OF_LINES - 1; i >= 0; i--) {
		if (strncmp(output_arr[i], "00000000", COMMAND_LEN) != 0) {
			last_ind = i;
			break;
		}
	}
	// print memout file. 
	for (i = 0; i <= last_ind; i++) {
		if (output_arr[i][0] == '\0') {
			strncpy(output_arr[i], "00000000", COMMAND_LEN);
		}
		output_arr[i][COMMAND_LEN] = '\n';
		if (fprintf(memout_p, "%s", output_arr[i]) < 0) {
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
void free_mem(char ***output_arr) {
	int i;

	if ((*output_arr) != NULL) {
		for (i = 0; i < MAX_NUM_OF_LINES; i++) {
			if ((*output_arr)[i] != NULL) {
				free((*output_arr)[i]);
			}

		}
		free(*output_arr);
	}
	return;
}