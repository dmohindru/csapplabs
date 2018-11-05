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
#include <time.h>

/* Block being represented as unsigned int which hold number of bytes */
typedef unsigned int block;

/* Single cache line */
typedef struct {
	clock_t LRU_flag;
	int tag;
} cache_line;



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
 * 8. Print results
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
	int set_bits, e_num, block_bits, cmd_options, print_usage_flag, verbose_flag, i, victim_line;
	int num_hits, num_miss, num_evictions;
	unsigned long address, set_field_mask, set, tag;
	clock_t current_least_used, previous_least_used = 0;
	/* Set various input bits to -1 as a flag if some input bits are not entered */
	set_bits = e_num = block_bits =  -1;
	
	num_hits = num_miss = num_evictions = 0;
	
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
				if (optarg == end) /* error in conversion */
					print_usage_flag = 1;
				break;
			case 'E':
				e_num = strtol(optarg, &end, 10);
				if (optarg == end) /* error in conversion */
					print_usage_flag = 1;
				break;
			case 'b':
				block_bits = strtol(optarg, &end, 10);
				if (optarg == end) /* error in conversion */
					print_usage_flag = 1;
				break;
			case 't':
				trace_file_str = optarg;
				break;
			default:
				print_usage_flag = 1;
		}
	}
	
	
	/* Print usage if as per various conditions */
	if (print_usage_flag  == -1 || set_bits == -1 || e_num == -1 || block_bits == -1 || trace_file_str == NULL)
	{
		
		printUsage();
		return 1;
	}
	
	cache_mem *cache_mem_ptr = createCacheMemory(set_bits, e_num, block_bits);
	if (!cache_mem_ptr)
		return 1;
	/* Debug stuff */
	//printf("Created cache memory\n");
		
	/* Open trace file for parsing */
	trace_file = fopen(trace_file_str, "r");
	if (trace_file == NULL)
	{
		perror(trace_file_str);
		return 1;
	}
	
	/* set field mask used in extracting set from address */
	set_field_mask = (unsigned)-1 >> (sizeof(long) * 8 - set_bits);
	//printf("set field mask: 0x%lX\n", set_field_mask);
	//int line_no = 0;
	/* Main loop to read trace file and simulate ram */
	while (fgets(line, sizeof(line), trace_file) != NULL)
	{
		/* Debug stuff */
		//line_no++;
		//if (strlen(line) <= 1)
		//	continue;
		//printf("fgets read line: %d\n", line_no);
		if (line[0] != ' ')
			continue;
			
		if (verbose_flag == 1)
		{
			line[strlen(line)-1] = '\0'; /* get rid of last \n character */
			printf("%s", line);
		}
		
		/* Extract operation */
		op = line[1];
		/* Debug stuff */
		//printf("operation extracted\n");
		
		/* Extract address */
		ptr = strtok(line+3, delimiter);
		address = strtol(ptr, &end, 16);
		/* Debug stuff */
		//printf("address extracted\n");
		if (ptr == end)
		{
			fclose(trace_file);
			printf("Error in address conversion at line %s", line);
			return 1;
		}
		
		/* Extract set and tag field */
		set = (address >> block_bits) & set_field_mask;
		tag = address >> (block_bits + set_bits);

		/* check cache memory */
		for (i = 0; i < e_num; i++)
		{
			if(cache_mem_ptr->cache_line_array[set][i].tag == tag)
			{
				/* Debug stuff */
				//printf("inside cache hit block\n");
				num_hits++;
				if (verbose_flag == 1)
					printf(" hit");
				if (op == 'M')
				{
					num_hits++;
					if (verbose_flag == 1)
						printf(" hit");
				}
				if (verbose_flag == 1)
					printf("\n");
				break;
			}
		}
		if (i == e_num) 
		{
			/* if reached here its a miss, evict a line as per
			   LRU policy and set new tag */ 
			/* Debug stuff */
			//printf("inside cache miss block\n");
			for (i = 0; i < e_num; i++)
			{
				if (cache_mem_ptr->cache_line_array[set][i].LRU_flag == 0)
				{
					/* unused line use this i */
					victim_line = -1;
					break; 
				}
				else
				{
					current_least_used = cache_mem_ptr->cache_line_array[set][i].LRU_flag; //record lru times	
					if (current_least_used <= previous_least_used || previous_least_used == 0)
					{
						previous_least_used = current_least_used;
						victim_line = i;
					}
				}
			}
			if (victim_line != -1)
			{
				i = victim_line;
				/* increment eviction here */
				num_evictions++;
				if (verbose_flag == 1)
					printf(" miss eviction");
			}
			else
			{
				if (verbose_flag == 1)
					printf(" miss");
			}
			/* increment miss here */
			num_miss++;
			cache_mem_ptr->cache_line_array[set][i].tag = tag;
			cache_mem_ptr->cache_line_array[set][i].LRU_flag = clock();
			if (op == 'M')
			{
				num_hits++;
				if (verbose_flag == 1)
					printf(" hit");
			}
			if (verbose_flag == 1)
				printf("\n");
		}
	}
	/* Debug stuff */
	//printf("end of simulation loop\n");
	//fclose(trace_file);
	/* Debug stuff */
	//printf("after fclose\n");
	free(cache_mem_ptr);
	/* Debug stuff */
	//printf("after free(cache_mem_ptr)\n");
    printSummary(num_hits, num_miss, num_evictions);
    return 0;
}
