
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define ARRAY_SIZE 10000  /* message size     */

int main( int argc, char *argv[] ) {
  /*  Processor 0 packs a send buffer and sends it off to processor 1.     */
  /*  Processor 1 receives this package and sends it off to                */
  /*  processor 2. This continues until processor 0 finally receives the   */
  /*  package and compares it with the original.                           */
  /*                                                                       */
  /*  Remember, it is convenient to reference the typical processor in the */
  /*  first person, using "I" and "me" and "my." In other words,           */
  /*  read through this program as if you were one of the processors.      */

  /*  My ID number and the total number of processors: */
  int myrank, numProcs;

  /*  Variables for send destination ID and receive source ID: */
  int dest, src;

  /*  Let's not use tagging. We're sending just one message. Set it to 1: */
  int tag = 1;

  /*  Allocate the array msg[] which will serve as both send and */
  /*  receive buffer:                                            */
  double msg[ARRAY_SIZE];

  /*  But if I am processor 0, I want a separate receive buffer for */
  /*  comparing with the original:                                  */
  double check[ARRAY_SIZE];

  /*  A couple of integer indices: */
  int i,proc;

  /*  And finally I need to declare an MPI status variable */
  /*  for checking the status of the MPI_Recv function:    */
  MPI_Status status;

  /*  Initialize the MPI API: */
  MPI_Init(&argc, &argv);

  /*  Request my ID number: */
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

  /*  I'll also ask how many other processors are out there: */
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

  /*  Okay. The preparations have been made. */

  /*  If I'm processor 0, then I should print the number of */
  /*  processors in this run. */
  if (myrank == 0) {
      printf( "The number of processors in this run is %d.\n", numProcs );
  }

  /*  If I'm processor 0, then I better get busy and pack my send buffer: */
  if (myrank == 0) {
      for (i = 0; i < ARRAY_SIZE; i++) {
          /*  I'll assign the array index as the value of array element */
          msg[i] = i;

	  /*  Also I better prepare my receive buffer: */
	  check[i] = 0;
      }
  }

  /*  Begin the round-robin: */
  for (proc = 0; proc < numProcs - 1; proc++) {
      /*  If I'm processor proc, then I want to send */
      /*  to processor proc + 1:                     */
      if (myrank == proc) {
	  dest = proc + 1;
	  MPI_Send( msg,
		    ARRAY_SIZE,
		    MPI_DOUBLE,
		    dest,
		    tag,
		    MPI_COMM_WORLD );
	  printf( "Processor %d has sent the message.\n", myrank );
      }
      /*  Else if I'm processor proc + 1, then I want to */
      /*  receive from processor proc:                   */
      else if (myrank == proc + 1) {
	  src = proc;
	  MPI_Recv( msg,
		    ARRAY_SIZE,
		    MPI_DOUBLE,
		    src,
		    tag,
		    MPI_COMM_WORLD,
		    &status );
	  printf( "Processor %d has received the message.\n", myrank );\
      }
  }

  /*  And if I happen to be the last processor */
  /*  then I want to send to processor 0:      */
  if (myrank == numProcs - 1) {
      dest = 0;
      MPI_Send( msg,
		ARRAY_SIZE,
		MPI_DOUBLE,
		dest,
		tag,
		MPI_COMM_WORLD );
      printf( "Processor %d has sent the message.\n", myrank );
  }

  /*  Finally if I'm processor 0 then I want to receive  */
  /*  from the last processor and compare this with what */
  /*  I'd originally sent:                               */
  if (myrank == 0) {
      src = numProcs - 1;
      MPI_Recv( check, 
		ARRAY_SIZE,
		MPI_DOUBLE,
		src, 
		tag, 
		MPI_COMM_WORLD,
		&status);

      for (i = 0; i < ARRAY_SIZE; i++) {
          if (msg[i] != check[i])  {
	      printf( "error in element %d of the message.\n", i );
          }
      }
      printf( "Processor %d has received and checked the message. "
              "Program terminating normally.\n", myrank );
  }

  /*  Close the MIP API: */
  MPI_Finalize();

  return 0;
}
