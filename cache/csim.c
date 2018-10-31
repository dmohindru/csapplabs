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

/* Implemenation steps
 * 1. gather command line arguments with error checking
 * 2. check for file being present specified by -t flag
 * 3. Decide data structure to represent cache with arbitrary 
 *    size of s, E, b.
 * 4. Parse trace file
 * 5. Simulate cache operations with miss, hit, evicit. Think of logic
 *    for evicition based on LRU (least recently used) policy.
 * 6. Implement verbose mode
 * 7. Print reults
 */
void print_usage()
{
	printf("Usage\n");
}
int main(int argc, char *argv[])
{ 
	char *end, *trace_file = 0;
	int set_bits, e_bits, block_bits, cmd_options, print_usage_flag, verbose_flag;
	set_bits = e_bits = block_bits =  -1;
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
				e_bits = strtol(optarg, &end, 10);
				if (optarg == end) //error in conversion
					print_usage_flag = 1;
				break;
			case 'b':
				block_bits = strtol(optarg, &end, 10);
				if (optarg == end) //error in conversion
					print_usage_flag = 1;
				break;
			case 't':
				trace_file = optarg;
				break;
			default:
				print_usage_flag = 1;
		}
	}
	if ((print_usage_flag  == 1) || set_bits == -1 || e_bits == -1 || block_bits == -1 || trace_file == 0)
	{
		print_usage();
		return 0;
	}
	if (verbose_flag == 1)
		printf("Running simulation in verbose mode\n");
	printf("Input values: s=%u, E=%u, b=%u, t=%s\n", set_bits, e_bits, block_bits, trace_file);
    printSummary(0, 0, 0);
    return 0;
}
