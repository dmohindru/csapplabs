/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
 /*
  * Cache configuration:-
  * Cache Size = 1024 bytes
  * Block Size = 32 bytes
  * Num of Sets = 1024 / 32 = 32 Sets
  * Block bits = log2(32) = 5 bits
  * Set bits = log2(32) = 5 bits
  * Remaining bits in address as tag bits
  * Now assuming sizeof(int) = 4 bytes
  * So sizeof int A[32][32] = 32 X 32 X 4 = 4096 bytes
  * i.e cache can hold at most 1024/4096 = 1/4 of array in memory at any time.
  * Read access patterns than can have good miss ration 
  * 1. Read array A in pattern as
  * 	A[0][0], A[0][1], A[0][2], A[0][3]... A[0][7]     ---> Block  (32 bytes)
  * 	A[1][0], A[1][1], A[1][2], A[1][3]... A[1][7]     ---> Block  (32 bytes)
  * 	.....
  * 	A[15][0], A[15][1], A[15][2], A[15][3]... A[15][7] ---> Block  (32 bytes) (Total: 512 bytes)
  * 
  * 2. Above read pattern can have array B write pattern as
  * 	B[0][0], B[0][1], ... B[0][15]     	
  * 	B[1][0], B[1][1], ... B[1][15]     	
  * 	B[2][0], B[2][1], ... B[2][15]     	
  * 	B[3][0], B[3][1], ... B[3][15]
  * 	.....
  * 	B[7][0], B[7][1], ... B[7][15]     	
  * 	
  */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}
char my_trans_desc[] = "My Test transpose function";
void my_trans(int M, int N, int A[N][M], int B[M][N])
{
}
/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 
    
    registerTransFunction(my_trans, my_trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

