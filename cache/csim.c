/* Name: Dhruv Mohindru
 * Email: dmohindru@gmail.com
 * Description: Simple cache simulator with arbitrary set size, associativity 
 *              and block size.
 */ 
#include "cachelab.h"
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/* Block being represented as unsigned int which hold number of bytes */
typedef unsigned char block;

/* Single cache line */
typedef struct {
	unsigned char LRU_flag;
	block block_line;
} cache_line;

/* Single cache set. MIGHT NOT NEED THIS.....
 */
cache_line *cache_set;

/* Complete cache memory. Size of cache memory calculated as
 * cache_memory = sizeof(cache_line) * associativity * num_of_sets 
 * And accessed as cache_memory[set_number][line_number]
 */ 
cache_line **cache_memory;

/* Implemenation steps
 * 1. gather command line arguments with error checking
 * 2. check for file being present specified by -t flag
 * 3. Decide data structure to represent cache with arbitrary 
 *    size of s, E, b.
 * 4. Parse trace file
 * 5. Simulate cache operations with miss, hit, evicit.
 * 6. Think of logic for evicition based on LRU (least recently used) policy.
 * 7. Implement verbose mode
 * 8. Print reults
 */

/* Function to create cache memory */
void create_cache_memory(int set_bits, int e_num, int block_bits)
{
	
}

/* Function to print program usage */
void print_usage()
{
	printf("./csim: Missing required command line argument\n");
	printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf(" -h\t\tPrint this help message.\n");
	printf(" -v\t\tOptional verbose flag.\n");
	printf(" -s <num>\tNumber of set index bits.\n");
	printf(" -E <num>\tNumber of lines per set.\n");
	printf(" -b <num>\tNumber of block offset bits.\n");
}
int main(int argc, char *argv[])
{ 
	// Various variable declerations
	// I think that line[128] declaratoin can be done in a better way
	char line[128], *end, *ptr, op, *trace_file_str = NULL;
	FILE *trace_file;
	char delimiter[] = ",";
	int set_bits, e_num, block_bits, cmd_options, print_usage_flag, verbose_flag, bytes;
	unsigned long address;
	// Set various input bits to -1 as a flag if some input bits are not entered
	set_bits = e_bits = block_bits =  -1;
	
	//Process command line arguments
	while ((cmd_options = getopt(argc, argv, "hvs:E:b:t:")) != -1)
	{
		switch(cmd_options)
		{
			case 'h':
				print_usage_flag = 1;
				break;
			case 'v':
				verbose_flag = 1;
				break;
			case 's':
				set_bits = strtol(optarg, &end, 10);
				if (optarg == end) //error in conversion
					print_usage_flag = 1;
				break;
			case 'E':
				e_num = strtol(optarg, &end, 10);
				if (optarg == end) //error in conversion
					print_usage_flag = 1;
				break;
			case 'b':
				block_bits = strtol(optarg, &end, 10);
				if (optarg == end) //error in conversion
					print_usage_flag = 1;
				break;
			case 't':
				trace_file_str = optarg;
				break;
			default:
				print_usage_flag = 1;
		}
	}
	
	// Print usage if as per various conditions
	if ((print_usage_flag  == 1) || set_bits == -1 || e_num == -1 || block_bits == -1 || trace_file_str == NULL)
	{
		print_usage();
		return 1;
	}
	
	//printf("Input values: s=%u, E=%u, b=%u, t=%s\n", set_bits, e_bits, block_bits, trace_file_str);
	
	// Open trace file for parsing
	trace_file = fopen(trace_file_str, "r");
	if (trace_file == NULL)
	{
		perror(trace_file_str);
		return 1;
	}
	
	// Main loop to read trace file and simulate ram
	while (fgets(line, sizeof(line), trace_file) != NULL)
	{
		if (verbose_flag == 1)
			printf("%s", line);
		if (line[0] != ' ')
			continue;
		op = line[1];
		printf("Operation: %c, ", op);
		ptr = strtok(line+3, delimiter);
		address = strtol(ptr, &end, 16);
		if (ptr == end)
		{
			fclose(trace_file);
			printf("Error in address conversion at line %s", line);
			return 1;
		}
		printf("Address: 0x%lx, ", address);
		ptr = strtok(NULL, delimiter);
		bytes = strtol(ptr, &end, 16);
		if (ptr == end)
		{
			fclose(trace_file);
			printf("Error in bytes conversion at line %s", line);
			return 1;
		}
		printf("Bytes: %d\n", bytes);
	}
	
	fclose(trace_file);
    printSummary(0, 0, 0);
    return 0;
}
