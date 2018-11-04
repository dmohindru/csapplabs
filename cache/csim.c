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
typedef unsigned int block;

/* Single cache line */
typedef struct {
	unsigned char LRU_flag;
	int tag;
} cache_line;

/* Single cache set. MIGHT NOT NEED THIS.....
 */
cache_line *cache_set;

/* Complete cache memory. Size of cache memory calculated as
 * cache_memory = sizeof(cache_line) * associativity * num_of_sets 
 * And accessed as cache_memory[set_number][line_number]
 */ 
typedef struct {
	unsigned int block_size;
	unsigned int set_size;
	unsigned int e_size;
	cache_line **cache_line_array;
} cache_mem;


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

/* Function to create cache memory and return a pointer to it.
 * If some thing goes wrong NULL is returned */
cache_mem  *createCacheMemory(int set_bits, int e_num, int block_bits)
{
	cache_mem *cache_mem_ptr = NULL;
	unsigned int block_size, set_size, i, j;
	
	/* Create cache memory */
	cache_mem_ptr = (cache_mem *) malloc(sizeof(cache_mem));
	if (!cache_mem_ptr)
	{
		printf("Error allocating cache memory structure\n");
		return NULL;
	}
	//printf("Created cache memory structure\n");
	//printf("sizeof cache_mem structure: %lu\n", sizeof(cache_mem));
	//printf("Allocated size of cache_mem structure: %lu\n", sizeof(cache_mem_ptr));
	/* Calculate block_size and set_size as 2^block_bits and 2^set_bits respectively */
	block_size = 1;
	for (i = 0; i < block_bits; i++)
	{
		block_size <<= 1;
	}
	
	set_size = 1;
	for (i = 0; i < set_bits; i++)
	{
		set_size <<= 1;
	}
	
	cache_mem_ptr->block_size = block_size;
	cache_mem_ptr->set_size = set_size;
	cache_mem_ptr->e_size = e_num;
	
	/* Now the real stuff. Allocate memory for cache line */
	/* Below code is a really cool stuff take from 
	   https://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/ */
	cache_mem_ptr->cache_line_array = (cache_line **)malloc(sizeof(cache_line *) * set_size);
	if(!cache_mem_ptr->cache_line_array)
	{
		printf("Error allocating cache line array\n");
		return NULL;
	}
	for (i = 0; i < set_size; i++)
	{
		cache_mem_ptr->cache_line_array[i] = (cache_line *)malloc(sizeof(cache_line) * e_num);
		if(!cache_mem_ptr->cache_line_array[i])
		{
			printf("Error allocating cache line\n");
			return NULL;
		}
	}
	
	/* Initalize cache_memory */
	for (i = 0; i < set_size; i++)
	{
		for (j = 0; j < e_num; j++)
		{
			cache_mem_ptr->cache_line_array[i][j].tag = -1;
			cache_mem_ptr->cache_line_array[i][j].LRU_flag = 0;
		}
	}
	return cache_mem_ptr;
	
}
/* Temp function for cache memory temp stuff */
void playWithCache(cache_mem *memory)
{
	unsigned int sets, e, i, j;
	sets = memory->set_size;
	e = memory->e_size;
	for (i = 0; i < sets; i++)
	{
		for (j = 0; j < e; j++)
		{
			memory->cache_line_array[i][j].tag = i + j;
			memory->cache_line_array[i][j].LRU_flag = j; 
		}
	}
	/* Display result */
	for (i = 0; i < sets; i++)
	{
		for (j = 0; j < e; j++)
		{
			printf("memory->cache_line_array[%u][%u].tag=%d\n",i, j, memory->cache_line_array[i][j].tag);
			printf("memory->cache_line_array[%u][%u].LRU_flag=%d\n",i, j, memory->cache_line_array[i][j].LRU_flag);
		}
	}
	
}
/* Function to print program usage */
void printUsage()
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
	/* Various variable declerations */
	/* I think that line[128] declaratoin can be done in a better way */
	char line[128], *end, *ptr, op, *trace_file_str = NULL;
	FILE *trace_file;
	char delimiter[] = ",";
	int set_bits, e_num, block_bits, cmd_options, print_usage_flag, verbose_flag, i;
	unsigned long address, set_field_mask, set, tag;
	/* Set various input bits to -1 as a flag if some input bits are not entered */
	set_bits = e_num = block_bits =  -1;
	
	/* Process command line arguments */
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
	
	//printf("Input values: s=%u, E=%u, b=%u, t=%s\n", set_bits, e_num, block_bits, trace_file_str);
	
	/* Print usage if as per various conditions */
	if (print_usage_flag  == -1 || set_bits == -1 || e_num == -1 || block_bits == -1 || trace_file_str == NULL)
	{
		
		printUsage();
		return 1;
	}
	
	cache_mem *cache_mem_ptr = createCacheMemory(set_bits, e_num, block_bits);
	if (!cache_mem_ptr)
		return 1;
	//playWithCache(cache_mem_ptr);
	/* Open trace file for parsing */
	trace_file = fopen(trace_file_str, "r");
	if (trace_file == NULL)
	{
		perror(trace_file_str);
		return 1;
	}
	
	
	
	/* print cache stats just a temp debug stuff */
	//printf("Cache set size: %u\n", cache_mem_ptr->set_size);
	//printf("Cache block size: %u\n", cache_mem_ptr->block_size);
	
	/* set field mask used in extracting set from address */
	set_field_mask = (unsigned)-1 >> (sizeof(long) * 8 - set_bits);
	//printf("set field mask: 0x%lX\n", set_field_mask);
	
	/* Main loop to read trace file and simulate ram */
	while (fgets(line, sizeof(line), trace_file) != NULL)
	{
		if (verbose_flag == 1)
		{
			line[strlen(line)-1] = '\0'; /* get rid of last \n character */
			printf("%s", line);
		}
		if (line[0] != ' ')
			continue;
		
		/* Extract operation */
		op = line[1];
		printf("Operation: %c, ", op);
		
		/* Extract address */
		ptr = strtok(line+3, delimiter);
		address = strtol(ptr, &end, 16);
		if (ptr == end)
		{
			fclose(trace_file);
			printf("Error in address conversion at line %s", line);
			return 1;
		}
		//printf("Address: 0x%lx, ", address);
		
		/* Extract bytes accessed MIGHT NOT NEED THIS */
		/*ptr = strtok(NULL, delimiter);
		bytes = strtol(ptr, &end, 16);
		if (ptr == end)
		{
			fclose(trace_file);
			printf("Error in bytes conversion at line %s", line);
			return 1;
		}
		printf("Bytes: %d\n", bytes); */
		
		/* Extract set and tag field */
		set = (address >> block_bits) & set_field_mask;
		tag = address >> (block_bits + set_bits);
		//printf("Extracted set field: %lx\n", set);
		//printf("Extracted tag field: %lx\n", tag);
		/* check cache memory */
		for (i = 0; i < e_num; i++)
		{
			if(cache_mem_ptr->cache_line_array[set][i].tag == tag)
			{
				printf("hit\n");
				break;
			}
		}
		if (i == e_num) 
		{
			/* if reached here its a miss, evict a line as per
			   LRU policy and set new tag */ 
			cache_mem_ptr->cache_line_array[set][0].tag = tag;
			printf("miss\n");
		}
		
		
	}
	
	fclose(trace_file);
	free(cache_mem_ptr);
    printSummary(0, 0, 0);
    return 0;
}
