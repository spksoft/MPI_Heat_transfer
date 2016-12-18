#define OUTPUT_FILE "output.txt"
#include "mpi.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
int getIndex(int rowIndex, int colIndex, int totalCol);
int generateNumber(int *numberArray, int row, int col);
void printArray(int *numberArray, int totalRow, int totalCol);
int *matrix_expand(int rows,int cols,int* matrix);
void generateDisplsAndCount(int *displs, int *count, int smallRow, int smallCol, int size);
void copyArray(int *firstArray, int *secondArray, int totalSize);
void WriteOutputFile(int *result, int row, int col);
void printArrayEx(int *matrix, int rows, int cols);
int main(int argc, char* argv[])
{
	int size, rank, totalSize, totalRow, totalCol, i, j, round, smallRow, smallCol, t_smallRow, t_smallCol, temp, Up, Left, Right, Down, t;
	int *numberArray, *smallNumberArray, *displs, *sizePerProcess, *forCal, *UpLine, *DownLine, *temp_forCal;
	double startTime, endTime;
	MPI_Request sendREQ,sendREQ1, recvREQ, recvREQ1;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	sendREQ = MPI_REQUEST_NULL;
	sendREQ1 = MPI_REQUEST_NULL;
	recvREQ = MPI_REQUEST_NULL;
	recvREQ1 = MPI_REQUEST_NULL;
	round = atoi(argv[3]);
	totalRow = atoi(argv[1]);
	totalCol = atoi(argv[2]);
	smallRow = (totalRow / 2) + (totalRow % 2);
	smallCol = (totalCol / 2) + (totalCol % 2);
	//numberArray = (int *)malloc(sizeof(int) * (totalRow * totalCol));
	smallNumberArray = (int *)malloc(sizeof(int) * (smallRow * smallCol));
	displs = (int *) malloc(sizeof(int) * size);
	sizePerProcess = (int *) malloc(sizeof(int) * size);
	generateDisplsAndCount(displs, sizePerProcess, smallRow, smallCol, size);
	forCal = (int *) malloc(sizeof(int) * sizePerProcess[rank]);
	temp_forCal = (int *) malloc(sizeof(int) * sizePerProcess[rank]);
	DownLine = (int *) malloc(sizeof(int) * smallCol);
	UpLine = (int *) malloc(sizeof(int) * smallCol);
	t_smallRow = sizePerProcess[rank] / smallCol;
	t_smallCol = smallCol;
	if(rank == 0 ) {
		generateNumber(smallNumberArray, smallRow, smallCol);
		startTime = MPI_Wtime();
		MPI_Scatterv(smallNumberArray, sizePerProcess, displs, MPI_INT, forCal, sizePerProcess[rank], MPI_INT, 0, MPI_COMM_WORLD);
		//printf("Rank : %d\n", rank);
		//printArray(forCal, t_smallRow, t_smallCol);
		//getchar();
		if(t_smallRow != 1) {
			for(t = 0;t < round; t++) {
				if(size > 1) {
					MPI_Isend(&forCal[getIndex(t_smallRow - 1, 0, t_smallCol)], t_smallCol, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &sendREQ);
					MPI_Irecv(DownLine, t_smallCol, MPI_INT, rank + 1, 0,MPI_COMM_WORLD,&recvREQ);
				}
				copyArray(forCal, temp_forCal, t_smallRow * t_smallCol);
				//printf("Temp value :\n");
				//printArray(temp_forCal, t_smallRow, t_smallCol);
				for(i = 1;i<t_smallRow;i++) {
					for(j = 1;j<t_smallCol;j++) {
						if(forCal[getIndex(i, j, t_smallCol)] != 255) {
							if(rank == size - 1 && i == t_smallRow - 1) {
								Up = temp_forCal[getIndex(i - 1, j, t_smallCol)];
								if((totalRow % 2) == 0) {
									Down = temp_forCal[getIndex(i, j, t_smallCol)];
								} else {
									Down = temp_forCal[getIndex(i - 1, j, t_smallCol)];
								}
								Left = temp_forCal[getIndex(i, j - 1, t_smallCol)];
								if(j + 1 >= t_smallCol) {
									if((totalCol % 2) == 0) {
										Right = temp_forCal[getIndex(i, j, t_smallCol)];
									} else {
										Right = temp_forCal[getIndex(i, j - 1, t_smallCol)];
									}
								} else {
									Right = temp_forCal[getIndex(i, j + 1, t_smallCol)];
								}
								//printf("Rank : %d\n", rank);
								//getchar();
								forCal[getIndex(i, j, t_smallCol)] = (Up + Down + Left + Right) / 4;
							} else {
								Up = temp_forCal[getIndex(i - 1, j, t_smallCol)];
								if(i + 1 >= t_smallRow) {
									MPI_Wait(&recvREQ,&status);
									//printf("Recv DownLine Form %d\n", rank + 1);
									//printArray(DownLine, 1, t_smallCol);
									Down = DownLine[j];
								} else {
									Down = temp_forCal[getIndex(i + 1, j, t_smallCol)];
								}
								Left = temp_forCal[getIndex(i, j - 1, t_smallCol)];
								if(j + 1 >= t_smallCol) {
									if((totalCol % 2) == 0) {
										Right = temp_forCal[getIndex(i, j, t_smallCol)];
									} else {
										Right = temp_forCal[getIndex(i, j - 1, t_smallCol)];
									}
								} else {
									Right = temp_forCal[getIndex(i, j + 1, t_smallCol)];
								}
								//printf("Rank : %d\n", rank);
								//getchar();
								forCal[getIndex(i, j, t_smallCol)] = (Up + Down + Left + Right) / 4;
							}
						}
					}
				}
				//printf("Rank : %d, T :%d\n", rank, t);
				//printArray(forCal, t_smallRow, t_smallCol);
				if(size > 1)
					MPI_Wait(&sendREQ,&status);
				printf("Rank : %d ; Loop : %d\n", rank, t);
			}
		} else {
			for(t = 0;t < round; t++) {
				MPI_Isend(&forCal[getIndex(t_smallRow - 1, 0, t_smallCol)], t_smallCol, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &sendREQ);
				MPI_Wait(&sendREQ,&status);
				printf("Rank : %d ; Loop : %d\n", rank, t);
			}
		}
		MPI_Gatherv(forCal, sizePerProcess[rank], MPI_INT, smallNumberArray, sizePerProcess, displs,MPI_INT, 0, MPI_COMM_WORLD);
		
		endTime = MPI_Wtime();
		numberArray = matrix_expand(totalRow, totalCol, smallNumberArray);
		//printArray(smallNumberArray, smallRow, smallCol);
		//printArray(numberArray, totalRow, totalCol);
		printf("%lf\n", endTime - startTime);
		printf("\n");
		WriteOutputFile(numberArray, totalRow, totalCol);
		
		getchar();
	} else {
		MPI_Scatterv(smallNumberArray, sizePerProcess, displs, MPI_INT, forCal, sizePerProcess[rank], MPI_INT, 0, MPI_COMM_WORLD);
		/*printf("Rank : %d\n", rank);
		printArray(forCal, t_smallRow, t_smallCol);
		getchar();*/
		for(t = 0;t < round; t++) {
			copyArray(forCal, temp_forCal, t_smallRow * t_smallCol);
			//printf("Temp value :\n");
			//printArray(temp_forCal, t_smallRow, t_smallCol);
			if(rank - 1 == 0 && sizePerProcess[rank - 1] / t_smallCol == 1) {
			}
			else {
				MPI_Isend(&forCal[getIndex(0, 0, t_smallCol)], t_smallCol, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &sendREQ); //SendUp to Prev
				
			}
			MPI_Irecv(UpLine, t_smallCol, MPI_INT, rank - 1, 0,MPI_COMM_WORLD,&recvREQ); // Recv UpLine from prev
			if(rank != size - 1) {
				MPI_Isend(&forCal[getIndex(t_smallRow - 1, 0, t_smallCol)], t_smallCol, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &sendREQ1); //SendUp to Next
				MPI_Irecv(DownLine, t_smallCol, MPI_INT, rank + 1, 0,MPI_COMM_WORLD,&recvREQ1); // Recv DownLine from next
			}
			for(i = 0;i<t_smallRow;i++) {
				for(j = 1;j<t_smallCol;j++) {
					if(forCal[getIndex(i, j, t_smallCol)] != 255) {
						if(rank == size - 1 && i == t_smallRow - 1) {
							MPI_Wait(&recvREQ,&status);
							//printf("Recv UpLine Form %d\n", rank - 1);
							//printArray(UpLine, 1, t_smallCol);
							Up = UpLine[j];
							if((totalRow % 2) == 0) {
								Down = temp_forCal[getIndex(i, j, t_smallCol)];
							} else {
								if(t_smallRow == 1) {Down = UpLine[j];}
								else
									Down = temp_forCal[getIndex(i - 1, j, t_smallCol)];
							}
							Left = temp_forCal[getIndex(i, j - 1, t_smallCol)];
							if(j + 1 >= t_smallCol) {
								if((totalCol % 2) == 0) {
									Right = temp_forCal[getIndex(i, j, t_smallCol)];
								} else {
									Right = temp_forCal[getIndex(i, j - 1, t_smallCol)];
								}
									
							} else {
								Right = temp_forCal[getIndex(i, j + 1, t_smallCol)];
							}
							//printf("Rank : %d\n", rank);
							//getchar();
							forCal[getIndex(i, j, t_smallCol)] = (Up + Down + Left + Right) / 4;
						} else {
							MPI_Wait(&recvREQ,&status);
							//printf("Recv UpLine Form %d\n", rank - 1);
							//printArray(UpLine, 1, t_smallCol);
							Up = UpLine[j];
							if(i + 1 >= t_smallRow) {
								MPI_Wait(&recvREQ1,&status);
								//printf("Recv DownLine Form %d\n", rank + 1);
								//printArray(DownLine, 1, t_smallCol);
								Down = DownLine[j];
							} else {
								Down = temp_forCal[getIndex(i + 1, j, t_smallCol)];
							}
							Left = temp_forCal[getIndex(i, j - 1, t_smallCol)];
							if(j + 1 >= t_smallCol) {
								if((totalCol % 2) == 0) {
									Right = temp_forCal[getIndex(i, j, t_smallCol)];
								} else {
									Right = temp_forCal[getIndex(i, j - 1, t_smallCol)];
								}
							} else {
								Right = temp_forCal[getIndex(i, j + 1, t_smallCol)];
							}
							//printf("Rank : %d\n", rank);
							//getchar();
							forCal[getIndex(i, j, t_smallCol)] = (Up + Down + Left + Right) / 4;
						}
					}
				}
			}
			//printf("Rank : %d, T :%d\n", rank, t);
			//printArray(forCal, t_smallRow, t_smallCol);
			if(rank != size -1) {
				MPI_Wait(&sendREQ1,&status);
			}
			if(rank - 1 == 0 && sizePerProcess[rank - 1] / t_smallCol == 1){}
			else {
				MPI_Wait(&sendREQ,&status);
			}
			//printf("Rank : %d ; Loop : %d\n", rank, t);
		}
		MPI_Gatherv(forCal, sizePerProcess[rank], MPI_INT, smallNumberArray, sizePerProcess, displs,MPI_INT, 0, MPI_COMM_WORLD);
	}
	MPI_Finalize();
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
	for(i = 0;i<totalSize;i++) {
		secondArray[i] = firstArray[i];
	}
}

void WriteOutputFile(int *result, int row, int col) {
	int i, j, count, total;
	FILE *fp;
	fp = fopen(OUTPUT_FILE, "w");
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
