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
  * 3. Access pattern (16X16)
  * A[0][0]-A[0][7](Block 0) ==> B[0][0]-B[7][0] (Block 0, 2, 4, 6, 8, 10, 12, 14)
  * A[1][0]-A[1][7] (Block x) ==> B[0][1]-B[7][1] (Block 0, 2, 4, 6, 8, 10, 12, 14)
  * ....
  * A[7][0]-A[7][7] (Block x)==> B[0][7]-B[7][7] (Block 0, 2, 4, 6, 8, 10, 12, 14)   	
  * ==========
  * A[0][8]-A[0][15] (Block x)==> B[8][0]-B[15][0] (Block 16, 18, 20, 22, 24, 26, 28, 30)
  * A[1][8]-A[1][15] (Block x)==> B[8][1]-B[15][1]
  * ....
  * A[7][8]-A[7][15] (Block x)==> B[8][7]-B[15][7]
  * 
  * 
  * Desired access pattern (16x16 matrix)
  * A[0][0]:A[0][7] => B[0][0]:B[7][0]
  * A[1][0]:A[1][7] => B[0][1]:B[7][1]
  * A[2][0]:A[2][7] => B[0][2]:B[7][2]
  * ......
  * A[7][0]:A[7][7] => B[0][7]:B[7][7]
  * ==================
  * A[0][8]:A[0][15] => B[8][0]:B[15][0]
  * A[1][8]:A[1][15] => B[8][1]:B[15][1]
  * A[2][8]:A[2][15] => B[8][2]:B[15][2]
  * ......
  * A[7][8]:A[7][15] => B[8][7]:B[15][7]	
  * ==================
  * A[8][0]:A[8][7] => B[0][8]:B[7][8]
  * A[9][0]:A[9][7] => B[0][9]:B[7][9]
  * A[10][0]:A[10][7] => B[0][10]:B[7][10]
  * ......
  * A[15][0]:A[15][7] => B[0][15]:B[7][15]
  * ==================
  * A[8][7]:A[8][15] => B[7][8]:B[15][8]
  * A[9][7]:A[9][15] => B[7][9]:B[15][9]
  * A[10][7]:A[10][15] => B[7][10]:B[15][10]
  * ......
  * A[15][7]:A[15][15] => B[7][15]:B[15][15]
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
void my_trans_64_64(int M, int N, int A[N][M], int B[M][N])
{
	 int i, j, k, l, t1, t2, t3, t4, t5, t6, t7, t8;
        for (i = 0; i < N; i += 8) {
            for (j = 0; j < M; j += 8) {
                for (k = i; k < i + 4; k++) {
                    t1 = A[k][j];
                    t2 = A[k][j + 1];
                    t3 = A[k][j + 2];
                    t4 = A[k][j + 3];
                    t5 = A[k][j + 4];
                    t6 = A[k][j + 5];
                    t7 = A[k][j + 6];
                    t8 = A[k][j + 7];

                    B[j][k] = t1;
                    B[j + 1][k] = t2;
                    B[j + 2][k] = t3;
                    B[j + 3][k] = t4;
                    B[j][k + 4] = t5;
                    B[j + 1][k + 4] = t6;
                    B[j + 2][k + 4] = t7;
                    B[j + 3][k + 4] = t8;
                }
                //this block of code seems misterious
                //need better explaination
                for (l = j + 4; l < j + 8; l++) {

                    t5 = A[i + 4][l - 4];
                    t6 = A[i + 5][l - 4];
                    t7 = A[i + 6][l - 4];
                    t8 = A[i + 7][l - 4];

                    t1 = B[l - 4][i + 4];
                    t2 = B[l - 4][i + 5];
                    t3 = B[l - 4][i + 6];
                    t4 = B[l - 4][i + 7];

                    B[l - 4][i + 4] = t5;
                    B[l - 4][i + 5] = t6;
                    B[l - 4][i + 6] = t7;
                    B[l - 4][i + 7] = t8;

                    B[l][i] = t1;
                    B[l][i + 1] = t2;
                    B[l][i + 2] = t3;
                    B[l][i + 3] = t4;

                    B[l][i + 4] = A[i + 4][l];
                    B[l][i + 5] = A[i + 5][l];
                    B[l][i + 6] = A[i + 6][l];
                    B[l][i + 7] = A[i + 7][l];
                }
            }
		}
}
void my_trans_67_61(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, l, block_size;
	block_size = 23; // why this mysterious number
	
	for (l = 0; l < N; l+=block_size)
	{
		for(k = 0; k < M; k+=block_size)
		{
			for (i = l; i < l + block_size && i < N; i++)
			{
				for (j = k; j < k + block_size && j < M; j++)
				{
					B[j][i] = A[i][j];
				}
			}
		}
	}
}

void my_trans_32_32(int M, int N, int A[N][M], int B[M][N])
{
	int i, j, k, l, block_size, r0, r1, r2, r3, r4, r5, r6, r7;
	block_size = 8; // 32 / 4
	
	for (l = 0; l < N; l+=block_size)
	{
		for(k = 0; k < M; k+=block_size)
		{
			for (i = l; i < l + block_size; i++)
			{
				for (j = k; j < k + block_size; j+=block_size)
				{
					r0 = A[i][j];
					r1 = A[i][j+1];
					r2 = A[i][j+2];
					r3 = A[i][j+3];
					r4 = A[i][j+4];
					r5 = A[i][j+5];
					r6 = A[i][j+6];
					r7 = A[i][j+7];
					
					B[j][i] = r0;
					B[j+1][i] = r1;
					B[j+2][i] = r2;
					B[j+3][i] = r3;
					B[j+4][i] = r4;
					B[j+5][i] = r5;
					B[j+6][i] = r6;
					B[j+7][i] = r7;
				}
			}
		}
	}
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
	if (M == 32 && N == 32) {
        //transpose_matrix_32_32(M, N, A, B);
        my_trans_32_32(M, N, A, B);
    }
    else if (M == 64 && N == 64) {
        my_trans_64_64(M, N, A, B);
        //trans(M, N, A, B);
    } else {
        //transpose_matrix_67_61(M, N, A, B);
        my_trans_67_61(M, N, A, B);
    }
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
    //registerTransFunction(trans, trans_desc); 
    
    //registerTransFunction(my_trans, my_trans_desc); 
    
    //registerTransFunction(sec_trans, second_trans_desc); 

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

