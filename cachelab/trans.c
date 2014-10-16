/*
 * trans.c - Matrix transpose B = A^T
 *
 * Name: Xin Li
 * Andrew ID: xinli1
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */

#include <stdio.h>
#include "cachelab.h"
#include "contracts.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/*
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. The REQUIRES and ENSURES from 15-122 are included
 *     for your convenience. They can be removed if you like.
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    REQUIRES(M > 0);
    REQUIRES(N > 0);
    
    // since we can only use 12 varibles,
    // so here I won't create a varible for block size
    // so there're some magic number in the code.
    int blockRow, blockCol, row, col;
    // I don't know if array is allow for variables.
    // just in case I use 8 variables, this seems a little stupid,
    // but I can live with it.
    int temp00, temp01, temp02, temp03, temp10, temp11, temp12, temp13;
    
    if (M == 32) {
        // the block size of 8 is good enough for M = 32
        for (blockRow = 0; blockRow < M; blockRow += 8) {
            for (blockCol = 0; blockCol < N; blockCol += 8) {
                for (row = blockRow; (row < blockRow + 8) && (row < M); row++){
                    for (col = blockCol; (col < blockCol+8)&&(col < N); col++){
                        if (row == col) {
                            temp00 = A[row][col];
                        }
                        else {
                            B[col][row] = A[row][col];
                        }
                    }
                    // here we deal with the trangle block.
                    // although we still can pass the test
                    // cases in a decent number of miss.
                    // but this makes the result better. 
                    if (blockRow == blockCol) {
                        B[row][row] = temp00;
                    }
                }
            }
        }
    }
    else if (M == 61) {
        /*
         * for not N*N matrix it's hard to find the best solution,
         * simly try a few sizes, and you will get the surprise
         * blockSize:   16      17      18      19      20      21
         * misses:      1847    1819    1815    1834    1837    1842
         * just try some block size and get some surprise result
         * in this case, we may pick block size as 18.
         */
        for (blockCol = 0; blockCol < N; blockCol += 18) {
            for (blockRow = 0; blockRow < M; blockRow += 18) {
                for (row = blockRow; (row < blockRow+18) && (row < M); row++){
                    for (col = blockCol; (col<blockCol+18) && (col<N); col++){
                        B[row][col] = A[col][row];
                    }
                }
                
            }
        }
    }
    /*
     * for this problem, first came in mind is 
     * we can try put in smaller submatrix.
     * but the best for small matrices is round 1600.
     * so it's not good enough.
     */
    else if(M == 64) {
        // the block size of shi problem is 8
        // and the sub block size is 4
        for (blockRow = 0; blockRow < N; blockRow += 8) {
            for (blockCol = 0; blockCol < M; blockCol += 8) {
                // first we need to consider the diagonal blocks
                // for the diagonal block, we access A and B
                // at the same row same time.
                if (blockCol == blockRow) {
                    for (row = blockRow; row < blockRow + 8; row += 4) {
                        // the conflic miss happens when
                        // we try to access the same row of
                        // A and B at the same time.
                        // so we can use variables to avoid this.
                        // the cold miss is unavoidable,
                        // but we can save all the info before
                        // we evict the block of A.
                        // this time we use the variable to record the A,
                        // and after that we put them in B,
                        // this can reduce the conflict miss.
                        for (col = blockCol; col < blockCol + 8; col += 4) {
                            // for the following simply you can different order.
                            // and you will get 1299 and 1331
                            // two different results.
                            temp00 = A[row][col];
                            temp01 = A[row][col + 1];
                            temp02 = A[row + 1][col];
                            temp03 = A[row + 1][col + 1];
                            temp10 = A[row + 2][col];
                            temp11 = A[row + 2][col + 1];
                            temp12 = A[row + 3][col];
                            temp13 = A[row + 3][col + 1];
                            
                            B[col][row] = temp00;
                            B[col + 1][row] = temp01;
                            B[col][row + 1] = temp02;
                            B[col + 1][row + 1] = temp03;
                            B[col][row + 2] = temp10;
                            B[col + 1][row + 2] = temp11;
                            B[col][row + 3] = temp12;
                            B[col + 1][row + 3] = temp13;
                            
                            // since each time we only can move half block
                            // so we may need to move 2 times.
                            temp00 = A[row][col + 2];
                            temp01 = A[row][col + 3];
                            temp02 = A[row + 1][col + 2];
                            temp03 = A[row + 1][col + 3];
                            temp10 = A[row + 2][col + 2];
                            temp11 = A[row + 2][col + 3];
                            temp12 = A[row + 3][col + 2];
                            temp13 = A[row + 3][col + 3];
                            
                            B[col + 2][row] = temp00;
                            B[col + 3][row] = temp01;
                            B[col + 2][row + 1] = temp02;
                            B[col + 3][row + 1] = temp03;
                            B[col + 2][row + 2] = temp10;
                            B[col + 3][row + 2] = temp11;
                            B[col + 2][row + 3] = temp12;
                            B[col + 3][row + 3] = temp13;
                        }
                    }
                }
                else {
                    // for the non-diagonal block
                    // this time we also need to use the 4*4 matrix,
                    // so we may need manually do the four iterate.
                    // since the four iteration are different,
                    // so this may not that cubersome.
                    // this iteration takes care of the up left sub block
                    for (row = blockRow; row < blockRow + 4; row++) {
                        for (col = blockCol; col < blockCol + 4; col++) {
                            B[col][row] = A[row][col];
                        }
                    }
                    // I want to save as more information as possible.
                    // so 8 variables, we can at most save 8 of them,
                    // the rest, we may want to read them again.
                    temp00 = A[blockRow][blockCol + 4];
                    temp01 = A[blockRow][blockCol + 5];
                    temp10 = A[blockRow + 1][blockCol + 4];
                    temp11 = A[blockRow + 1][blockCol + 5];
                    temp02 = A[blockRow][blockCol + 6];
                    temp03 = A[blockRow][blockCol + 7];
                    temp12 = A[blockRow + 1][blockCol + 6];
                    temp13 = A[blockRow + 1][blockCol + 7];
                    
                    // this iteration takes care of
                    // the up right sub block of B
                    for (row = blockRow + 4; row < blockRow + 8; row++) {
                        for (col = blockCol; col < blockCol + 4; col++) {
                            B[col][row] = A[row][col];
                        }
                    }
                    
                    // this iteration takes care of
                    // the down right sub block of B
                    for (row = blockRow + 4; row < blockRow + 8; row++) {
                        for (col = blockCol + 4; col < blockCol + 8; col++){
                            B[col][row] = A[row][col];
                        }
                    }
                    
                    // in this situation,
                    // we can put back the information in the variables
                    // into B, before B evict.
                    B[blockCol + 4][blockRow] = temp00;
                    B[blockCol + 5][blockRow] = temp01;
                    B[blockCol + 4][blockRow + 1] = temp10;
                    B[blockCol + 5][blockRow + 1] = temp11;
                    B[blockCol + 6][blockRow] = temp02;
                    B[blockCol + 7][blockRow] = temp03;
                    B[blockCol + 6][blockRow + 1] = temp12;
                    B[blockCol + 7][blockRow + 1] = temp13;
                    // but we still need the rest two lines,
                    // so these conflict misses cannot be avoided.
                    for (row = blockRow + 2; row < blockRow + 4; row++) {
                        for (col = blockCol + 4; col < blockCol + 8; col++){
                            B[col][row] = A[row][col];
                        }
                    }
                }
            }
        }
    }
    
    ENSURES(is_transpose(M, N, A, B));
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

/*
 * trans - A simple baseline transpose function, 
 * not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;
    
    REQUIRES(M > 0);
    REQUIRES(N > 0);
    
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
    
    ENSURES(is_transpose(M, N, A, B));
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

