// heat2.cpp : Defines the entry point for the console application.
//
#include "mpi.h"
#include "omp.h"
#include <stdlib.h>
#include <time.h>

int getIndex(int rowIndex, int colIndex, int totalCol);
int generateNumber(int *numberArray, int row, int col);
void printArray(int *numberArray, int totalRow, int totalCol);
int *matrix_expand(int rows,int cols,int* matrix);
void generateDisplsAndCount(int *displs, int *count, int smallRow, int smallCol, int size);
void copyArray(int *firstArray, int *secondArray, int totalSize);
void WriteOutputFile(int *result, int row, int col);
void printArrayEx(int *matrix, int rows, int cols);

int _tmain(int argc, char* argv[])
{
	int totalSize, totalRow, totalCol, i, j, t, round, smallRow, smallCol, Up, Down, Left, Right;
	int *numberArray, *forCal, *temp_forCal;
	double startTime, endTime;
	round = atoi(argv[3]);
	totalRow = atoi(argv[1]);
	totalCol = atoi(argv[2]);
	smallRow = (totalRow / 2) + (totalRow % 2);
	smallCol = (totalCol / 2) + (totalCol % 2);
	numberArray = (int *)malloc(sizeof(int) * (totalRow * totalCol));
	forCal = (int *)malloc(sizeof(int) * (smallRow * smallCol));
	temp_forCal = (int *)malloc(sizeof(int) * (smallRow * smallCol));
	generateNumber(forCal, smallRow, smallCol);
	//startTime = omp_get_wtime();
	for(t = 0;t < round;t++) {
		copyArray(forCal, temp_forCal, smallRow * smallCol);
		#pragma omp parallel for private(j)
		for(i = 1;i<smallRow;i++) {
			for(j = 1;j<smallCol;j++) {
				if(forCal[getIndex(i, j, smallCol)] != 255) {
					Up = temp_forCal[getIndex(i - 1, j, smallCol)];
					if(i + 1 >= smallRow) {
						if((totalRow % 2) == 0) {
							Down = temp_forCal[getIndex(i, j, smallCol)];
						}
						else {
							Down = temp_forCal[getIndex(i - 1, j, smallCol)];
						}
					} else {
						Down = temp_forCal[getIndex(i + 1, j, smallCol)];
					}
					Left = temp_forCal[getIndex(i, j - 1, smallCol)];
					if(j + 1 >= smallCol) {
						if((totalCol % 2) == 0) {
							Right = temp_forCal[getIndex(i, j, smallCol)];
						}
						else {
							Right = temp_forCal[getIndex(i, j - 1, smallCol)];
						}
					}
					else {
						Right = temp_forCal[getIndex(i, j + 1, smallCol)];
					}
					forCal[getIndex(i, j, smallCol)] = (Up + Down + Left + Right) >> 2;
				}
			}
		}
	}
	//endTime = omp_get_wtime();
	//printf("Time : %lf\n", endTime - startTime);
	numberArray = matrix_expand(totalRow, totalCol, forCal);
	WriteOutputFile(numberArray, totalRow, totalCol);
	printf("Done\n");
	getchar();
	return 0;
}

int generateNumber(int *numberArray, int row, int col) {
	int i, j;
	srand(time(NULL));
	for(i = 0;i<row;i++) {
		for(j = 0;j<col;j++) {
			if(i == 0 || j == 0) {
				numberArray[getIndex(i,j,col)] = 255;
			}
			else {
				numberArray[getIndex(i,j,col)] = 0;
			}
		}
	}
	return 0;
}
int getIndex(int rowIndex, int colIndex, int totalCol) {
	int result = (rowIndex * totalCol) + colIndex;
	return result;
}
void printArray(int *numberArray, int totalRow, int totalCol) {
	int i, j;
	for(i = 0;i<totalRow;i++) {
		for(j = 0;j<totalCol;j++) {
			printf("%d\t", numberArray[getIndex(i, j, totalCol)]);
		}
		printf("\n");
	}
}
void printArrayEx(int *matrix, int rows, int cols) {
	int i, count = 1;
	for(i = 0;i<rows * cols;i++) {
		printf("%d ", matrix[i]);
		count++;
		if(count == cols) {
			count = 1;
			printf("\n");
		}
	}
}
int *matrix_expand(int rows,int cols,int* matrix) {
	int rows_divided = (rows/2 + rows%2);
	int cols_divided = (cols/2 + cols%2);
	int rows_cols_divided = rows_divided * cols_divided;
	int rows_cols_full = rows * cols;
	int i,j,k;

	int *full_matrix = (int*) malloc(sizeof(int) * rows_cols_full);
	for(i = 0,j = 0,k = cols-1; i < rows_cols_divided; i++,j++,k--) {
		if(i % cols_divided == 0 && i != 0) {
			j = j + cols_divided;
			k = k + cols_divided + cols;
			if(cols%2) {
				j -= 1;
			}
		}
		full_matrix[j] = matrix[i];
		full_matrix[k] = matrix[i];
		full_matrix[(rows_cols_full-1) - j] = matrix[i];
		full_matrix[(rows_cols_full-1) - k] = matrix[i];
	}
	return full_matrix;
}
void generateDisplsAndCount(int *displs, int *count, int smallRow, int smallCol, int sizeProcess) {
	int i, firstPosition = 0;
	long rowPerProcess = (long) (smallRow / sizeProcess);
	int modNumber =  smallRow % sizeProcess;
	for(i = 0;i<sizeProcess;i++) {
		if(modNumber != 0) {
			count[i] = (rowPerProcess + 1) * smallCol;
			displs[i] = firstPosition;
			firstPosition = firstPosition + count[i];
			modNumber--;
		} else {
			count[i] = (rowPerProcess ) * smallCol;
			displs[i] = firstPosition;
			firstPosition = firstPosition + count[i];
		}
	}
}
void copyArray(int *firstArray, int *secondArray, int totalSize) {
	int i;
	#pragma omp parallel for
	for(i = 0;i<totalSize;i++) {
		secondArray[i] = firstArray[i];
	}
}

void WriteOutputFile(int *result, int row, int col) {
	int i, j, count, total;
	FILE *fp;
	fp = fopen("output.txt", "w");
	total = row * col;
	count = 0;
	for(i = 0;i<total;i++) {
		if(count == col){ fprintf(fp, "\n"); count = 0; }
		fprintf(fp, "%d\t", result[i]);
		count++;
	}
	fprintf(fp, "\n");
	fclose(fp);
}


