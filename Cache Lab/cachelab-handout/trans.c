/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"



int min(int a, int b){

return a < b ? a : b;
}
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
   	int i, j, x, y;
	int _0, _1, _2, _3, _4, _5, _6, _7;
	int tmp;

	if(N == 32){ 
		for (i = 0; i < 4; i++) {
    	    for (j = 0; j < 4; j++) {
				if(i != j){
					for (x = 0; x < 8; x ++){
						for (y = 0; y < 8; y ++){
						B[j*8 + y][i*8 + x] = A[i*8 + x][j*8 + y];
						}
					}
				}else{
					for (x = 0; x < 8; x ++){
						_0 = A[i*8 + x][j*8];
						_1 = A[i*8 + x][j*8 + 1];
						_2 = A[i*8 + x][j*8 + 2];
						_3 = A[i*8 + x][j*8 + 3];
						_4 = A[i*8 + x][j*8 + 4];
						_5 = A[i*8 + x][j*8 + 5];
						_6 = A[i*8 + x][j*8 + 6];
						_7 = A[i*8 + x][j*8 + 7];
						
						B[i*8 + x][j*8] = _0;
						B[i*8 + x][j*8 + 1] = _1;
						B[i*8 + x][j*8 + 2] = _2;
						B[i*8 + x][j*8 + 3] = _3;
						B[i*8 + x][j*8 + 4] = _4;
						B[i*8 + x][j*8 + 5] = _5;
						B[i*8 + x][j*8 + 6] = _6;
						B[i*8 + x][j*8 + 7] = _7;
						}
					for (x = 0; x < 8; x ++){
						for (y = x; y < 8; y ++){
						tmp = B[i*8 + x][j*8 + y];
						B[i*8 + x][j*8 + y] = B[j*8 + y][i*8 + x];
						B[j*8 + y][i*8 + x] = tmp;
						}
					}

				}
    	    }
    	}
	}
	else if(N == 64){
		for (i = 0; i < 8; i++) {
    	    for (j = 0; j < 8; j++) {
/*
				for (x = 0; x < 4; x ++){
					for (y = 0; y < 4; y ++){
					B[j*8 + y][i*8 + x] = A[i*8 + x][j*8 + y];
					}
					for (y = 4; y < 8; y ++){
					B[j*8 + y - 4][i*8 + x + 4] = A[i*8 + x][j*8 + y];
					}
				}*/
				for (x = 0; x < 4; x ++){
						_0 = A[i*8 + x][j*8];
						_1 = A[i*8 + x][j*8 + 1];
						_2 = A[i*8 + x][j*8 + 2];
						_3 = A[i*8 + x][j*8 + 3];
						_4 = A[i*8 + x][j*8 + 4];
						_5 = A[i*8 + x][j*8 + 5];
						_6 = A[i*8 + x][j*8 + 6];
						_7 = A[i*8 + x][j*8 + 7];
						
						B[j*8 + 0][i*8 + x] = _0;
						B[j*8 + 1][i*8 + x] = _1;
						B[j*8 + 2][i*8 + x] = _2;
						B[j*8 + 3][i*8 + x] = _3;
						B[j*8 + 0][i*8 + x + 4] = _4;
						B[j*8 + 1][i*8 + x + 4] = _5;
						B[j*8 + 2][i*8 + x + 4] = _6;
						B[j*8 + 3][i*8 + x + 4] = _7;
				}


				for(y = 0; y < 4; y ++){
					_0 = B[j*8 + y][i*8 + 4];	
					_1 = B[j*8 + y][i*8 + 5];
					_2 = B[j*8 + y][i*8 + 6];
					_3 = B[j*8 + y][i*8 + 7];
					for(x = 4; x < 8; x++){
						B[j*8 + y][i*8 + x] = A[i*8 + x][j*8 + y];
					}
					B[j*8 + y + 4][i*8] = _0;
					B[j*8 + y + 4][i*8 + 1] = _1;
					B[j*8 + y + 4][i*8 + 2] = _2;
					B[j*8 + y + 4][i*8 + 3] = _3;
				}


				for(x = 4; x < 8; x++){
					for(y = 4; y < 8; y ++){
						B[j*8 + y][i*8 + x] = A[i*8 + x][j*8 + y];
					}	
				}




			}
				
				//for (x = 0; x < 8; x ++){
				/*	for (y = 0; y < 8; y ++){
					tmp[y] = A[i*8 + x][j*8 + y];
					}*/
				//	for (y = 0; y < 8; y ++){
				//	B[j*8 + y][i*8 + x] = A[i*8 + x][j*8 + y];
				//	}
				//}
    	}
	}
else{
		int step = 18;
		for(i = 0; i < N; i += step){
			for(j = 0; j < M; j += step){
				for(x = i ; x < (i + step) && x < N; x ++){
					for (y = j; y < (j + step) && y < M; y ++){
					B[y][x] = A[x][y];
					}
				}
			}
		}
	}

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
/*
char trans1_desc[] = "Trans_1";
void trans1(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}
*/

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
//    registerTransFunction(trans1, trans1_desc); 

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

