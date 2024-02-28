#include "mpi.h"
#include <stdio.h>
#include "mmio.h"
#include "mmio.c"
#include <stdlib.h>
// global rank variable
int my_rank;
int main(int argc,char *args[])

{
	int size, rank;

	MPI_Init(&argc, &args);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	//  mmio test
	MM_typecode matcode;
	int matn, matm, nzeros=0;
	int  pimatm=0;
	int i,n;

	FILE *f = fopen("nos3.mtx", "r");
	if (rank==0)
	{
		mm_read_banner(f, &matcode);
		printf("  point 1");
		mm_read_mtx_crd_size(f, &pimatm, &matn, &nzeros);
		printf("pimatm=%d, matn=%d, nzeros=%d\n", pimatm, matn, nzeros);
	};

	int sendn = (int) pimatm;
	MPI_Bcast(&sendn, 1, MPI_INT, 0, MPI_COMM_WORLD);
	n = (int) sendn;
	if (rank == 0) printf("size=%d\n", size);
	printf("myrank=%d, pimatm=%d\n", rank, sendn);

	/*  calculate, how many rows must be send to each process    */
	/*  nrrpp  nr of rows per process   */

	int nrrpp,nel;

	nrrpp=matn/size;
	nel=nzeros/size;
	MPI_Bcast(&nrrpp, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&nel, 1, MPI_INT, 0, MPI_COMM_WORLD);
	printf(" each process will obtain %d   rows  nd at least %d elements\n",nrrpp,nel   );

	/* Czytanie macierzy z pliku */
	int sizeA=2*nel;
	double A[sizeA];
	int rowsA[sizeA],colA[sizeA];
	int sendA=0;

	/* first we read part of the matrix for   process 0  */

	int inrow, incol;
	double inval;
	i=0;
	if (rank==0)
	{
		fscanf(f, "%d %d %lg\n", &inrow, &incol, &inval);
		while (inrow<= nrrpp  )
		{
			rowsA[i]=inrow;
			colA[i]=incol;
			A[i]=inval;

			i++;
			fscanf(f, "%d %d %lg\n", &inrow, &incol, &inval);

			// printf("A: nzero=%d,   inrow=%d,    incol=%d,   inval=%lg\n", i, inrow,incol, inval);
		};
	};
	/*  now we read and send the lines to  processes 1 till n-2  */
	double B[sizeA];
	int rowsB[sizeA],colB[sizeA];
	int sendB=0;
	int proc,j,tag=1;
	int nrrpp1,  gnrrl;
	/*  tp which line we rad
	 global nr of read lines
	 */

	gnrrl=i;

	/*  And finally I need to declare an MPI status variable */
	/*  for checking the status of the MPI_Recv function:    */
	MPI_Status status;
	int rsend = 0;
	for(proc=1; proc<size-1; proc++)
	{
		i=0;
		if (rank==0)
		{
			nrrpp1=(proc+1)*nrrpp;
			while ( inrow <= nrrpp1  )
			{
				rowsB[i]=inrow;
				colB[i]=incol;
				B[i]=inval;
				//  printf("B: nzero=%d,   inrow=%d,    incol=%d,   inval=%lg\n", i, inrow,incol, inval);
				i++;
				fscanf(f, "%d %d %lg\n", &inrow, &incol, &inval);
			};
			i--;
			gnrrl=+i;
			// /*
			MPI_Send(&i,1,MPI_INT,proc,tag,MPI_COMM_WORLD);
			MPI_Send(B,i,MPI_DOUBLE,proc,tag,MPI_COMM_WORLD);
			MPI_Send(rowsB,i,MPI_INT,proc,tag,MPI_COMM_WORLD);
			MPI_Send(colB,i,MPI_INT,proc,tag,MPI_COMM_WORLD);
			//   */
			rsend = rsend + i;
			printf( "INITIAL!!: Processor %d has sent the message to %d with %d values.\n", rank, proc, i );
		}
		else if (proc == rank)
		{
			//  /*
			MPI_Recv(&i,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
			MPI_Recv(A,i,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
			MPI_Recv(rowsA,i,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
			MPI_Recv(colA,i,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
			// */
			printf( "INITIAL!!: Processor %d has received the message with %d values.\n", rank,i );
			// */
		};
	};

	//MPI_Barrier(MPI_COMM_WORLD);
	/*  we must send the remaining rows to the last process  */
	i=0;
	if (rank==0)
	{
		nrrpp1=(size)*nrrpp;
		printf("nzeros: %d gnrrl: %d nrrpp1:%d nrrpp: %d\nGetting remaining %d rows\n", nzeros, gnrrl, nrrpp1, nrrpp, nzeros-gnrrl); 
		//nzeros: 8402 gnrrl: 1728 nrrpp1:960 nrrpp: 192
		for(i=1;i<nzeros-rsend; i++)
		{
			rowsB[i]=inrow;
			colB[i]=incol;
			B[i]=inval;
			//  printf("B: nzero=%d,   inrow=%d,    incol=%d,   inval=%lg\n", i, inrow,incol, inval);
			fscanf(f, "%d %d %lg\n", &inrow, &incol, &inval);
		};
		i--;
		// /*
		proc = size-1;
		MPI_Send(&i,1,MPI_INT,proc,tag,MPI_COMM_WORLD);
		MPI_Send(B,i,MPI_DOUBLE,proc,tag,MPI_COMM_WORLD);
		MPI_Send(rowsB,i,MPI_INT,proc,tag,MPI_COMM_WORLD);
		MPI_Send(colB,i,MPI_INT,proc,tag,MPI_COMM_WORLD);
		//   */
		printf( "FINAL!!: Processor %d has sent the message to %d with %d values.\n", rank, proc, i );
	}
	else if (rank == size-1)
	{
		printf("myrank=%d last receive", rank);
		//  /*
		MPI_Recv(&i,1,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
		MPI_Recv(A,i,MPI_DOUBLE,0,tag,MPI_COMM_WORLD,&status);
		MPI_Recv(rowsA,i,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
		MPI_Recv(colA,i,MPI_INT,0,tag,MPI_COMM_WORLD,&status);
		// */
		printf( "FINAL!!: Processor %d has received the message with %d values.\n", rank,i );
		// */
	};

	printf("Process %d before finalizing\n", rank);
	MPI_Finalize();
	fclose(f);
	return  0;
}