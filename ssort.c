/* Parallel sample sort
 */
#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#define ROOT -1
#define TARGET -1


static int compare(const void *a, const void *b);


int main( int argc, char *argv[]) {

  int rank, nTasks;
  int i;
  int *locData;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nTasks);

  /* Number of random numbers per processor (this should be increased
   * for actual tests or could be passed in through the command line */
  int locDataSize =  100; // set originally to N = 100

  locData = calloc(locDataSize , sizeof(int));
  
  /* seed random number generator differently on every core */
  srand((unsigned int) (rank + 393919));

  /* fill vector with random integers */
  for (i = 0; i < locDataSize; ++i) {
    locData[i] = rand();
  }

  /* sort locally */
  qsort(locData, locDataSize, sizeof(int), compare);


  /////////////////////////////////////////////////////////////
  //////////////////////// STEP 1 /////////////////////////////
  /////////////////////////////////////////////////////////////
  /* randomly sample nSplitters entries from vector or select local splitters,
   * i.e., every N/P-th entry of the sorted vector */

  // number of splitters
  int nSplitters = 100;
  int jump = locDataSize/nSplitters;
  
  //create array of splitters
  int *splitters = calloc(nSplitters, sizeof(int));
  for( i = 0 ; i < nSplitters ; i++) {
    splitters[i] = locData[i*jump];
  }

  // print local data to screen (TEST) 
  if ( rank == ROOT ) {
    printf("Local data of %d:\n" , rank);
    for(i = 0 ; i < locDataSize ; i++ ) {
      printf("locData[%d] =  %d\n", i, locData[i]);
    }
    printf("\n\n");
    for(i = 0 ; i < nSplitters ; i++ ) {
      printf("splitters[%d] =  %d\n", i, splitters[i]);
    }
    printf("\n\n");
  }    












  /* every processor communicates the selected entries
   * to the root processor; use for instance an MPI_Gather */
  
  int rootDataSize = nTasks*nSplitters; 
  int * rootData = calloc(rootDataSize , sizeof(int));
  

  // here's the syntax for this command
  /* MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
               void *recvbuf, int recvcount, MPI_Datatype recvtype,
               int root, MPI_Comm comm)*/
  MPI_Gather(splitters, nSplitters, MPI_INT, rootData, nSplitters, MPI_INT, 0, MPI_COMM_WORLD);
  
  
   // print the splitters transmitted to root (TEST)
  if ( rank == ROOT ) {
    printf("Splitters at root node are:\n");
    for(i = 0 ; i < rootDataSize ; i++ ) {
      printf("rootData[%d] =  %d\n", i, rootData[i]);
    }
    printf("\n\n");

  }    
















  /* root processor does a sort, determines splitters that
   * split the data into P buckets of approximately the same size */

    
  qsort(rootData, rootDataSize , sizeof(int), compare);
  
  // print sorted splitters at root  (TEST)
  if ( rank == ROOT ) {
    printf("The sorted splitters at the root are:\n");
    for(i = 0 ; i < rootDataSize ; i++ ) {
      printf("rootData[%d] = %d\n", i, rootData[i]);
    }
    printf("\n\n");
  } 
   
  // the number of splitters in the list of the root
  int rootNumSplitters = nTasks - 1;
    
  // the jump in the list of the root
  int rootJump =rootDataSize /(rootNumSplitters+1);

  // create the root list of splitters
  int *rootSplitters = calloc(rootNumSplitters, sizeof(int));
  for( i = 0 ; i < rootNumSplitters ; i++) {
    rootSplitters[i] = rootData[(i+1)*rootJump];
  }

  // print the start\endpoints of buckets to screen (TEST)
  if ( rank == ROOT ) {
    printf("Buckets are between the following points:\n");
    for(i = 0 ; i < rootNumSplitters ; i++ ) {
      printf("rootSplitters[%d] =  %d\n", i, rootSplitters[i]);
    }
    printf("\n\n");
  }    










  /* root process broadcasts splitters */

  // the syntax of the following command
  /*MPI_Bcast( void *buffer, int count, MPI_Datatype datatype, int root, 
	     MPI_Comm comm )*/
  MPI_Bcast( rootSplitters, rootNumSplitters, MPI_INT, 0, MPI_COMM_WORLD );


  // print the received data  to screen (TEST)
  if ( rank == TARGET  ) {
    printf("Processor %d received the data:\n", rank);
    for(i = 0 ; i < rootNumSplitters ; i++ ) {
      printf("rootSplitters[%d] =  %d\n", i, rootSplitters[i]);
    }
    printf("\n\n");
  }    











  /* every processor uses the obtained splitters to decide
   * which integers need to be sent to which other processor (local bins) */
  
  // allocate memory for an array that holds the left and right endpoints of this
  // processor's data that goes to the ith processor
  int * indices  = calloc(2*nTasks , sizeof(int));

  // j indexes the splitters
  int j = 0;

  // go over all local data, compare it to splitters and do stuff:
  for( i = 0 ; i < locDataSize ; i++) {
    


    // whenever we find ourselves above the current splitter...
    if ( locData[i] >  rootSplitters[j] ) {
      
      // If this is the first splitter, do something special
      if ( j == 0 ) {
	indices[0] = 0;
	indices[1] = i;
      } 

      
      // if this is the last splitter, do something special
      else if ( j == nTasks - 1 ) {
	indices[2*nTasks - 2] = indices[2*nTasks-3]+1; // set startpoint of cell as 1 above previous endpoint
	indices[2*nTasks - 1] = locDataSize-1; // end cell at the ennd of the data
	break; // and break, since we have no more splitters
      }

      
      // o.w. - set start and end by rules below
      else {
	indices[2*j]  = indices[2*j-1]+1; // set startpoint of cell as 1 above previous endpoint
	indices[2*j+1]  = i-1; // set current endpoint to be previous point
      }


      j++; // move to index the next splitter
    }
  }


  
  // print the buckets endpoints (TEST)
  //if ( rank == ROOT  ) {
    /*
    printf("Processor %d has data :\n", rank);
    for(i = 0 ; i < locDataSize ; i++ ) {
      printf("locData[%d] =  %d\n", i, locData[i]);
    }
    printf("\n\n");
    */

    for( i = 0;  i < nTasks ; i++) {
      printf("%d -> %d : %d to %d\n",rank, i, indices[2*i], indices[2*i+1] );
    }
    //printf("\n\n");
    //}    
















  /* send and receive: either you use MPI_AlltoallV, or
   * (and that might be easier), use an MPI_Alltoall to share
   * with every processor how many integers it should expect,
   * and then use MPI_Send and MPI_Recv to exchange the data */


  int * gatheredIndices   = calloc(2*nTasks , sizeof(int) );

  // Syntax for the following command
  /*MPI_Alltoall(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
                 void *recvbuf, int recvcount, MPI_Datatype recvtype,
                 MPI_Comm comm)*/
  MPI_Alltoall(indices, 2, MPI_INT, gatheredIndices, 2, MPI_INT, MPI_COMM_WORLD);

  
  // print what a processor needs to get (TEST)
  //if ( rank == TARGET  ) {
    for(i = 0 ; i < nTasks ; i++ ) {
      printf("%d ->  %d : %d to %d\n", i, rank, gatheredIndices[2*i], gatheredIndices[2*i+1] );
    }
    //printf("\n\n");
    //}    


  // calculate memory to be allocated:
  int gatheredDataSize = 0;
  for(i = 0 ; i < nTasks ; i++ ) {
    gatheredDataSize +=   gatheredIndices[2*i+1] -  gatheredIndices[2*i] +1;
  }






  // allocate the memory
  int * gatheredData = calloc(gatheredDataSize , sizeof(int));
  MPI_Request sendReqs[4];
  MPI_Status sendStats[4];
  MPI_Request recReqs[4];
  MPI_Status recStats[4];
  

  int * sendStart; // pointer to the start of a send buffer
  int sendCount; // how many ints are to be sent
 
  int * recStart; // pointer to the start of the receive  buffer
  int recCount; // how many ints are to be received
  
  // the sends and receives
  for( i = 0 ; i < nTasks ; i++) {

    sendStart = &locData[ indices[2*i] ];
    sendCount = indices[2*i+1] - indices[2*i] + 1; 

    // syntax
    /*MPI_Isend(const void *buf, int count, MPI_Datatype datatype, int dest, int tag,
      MPI_Comm comm, MPI_Request *request)*/
    MPI_Isend(sendStart, sendCount, MPI_INT, i, rank, MPI_COMM_WORLD ,&sendReqs[i] );



    recStart = &gatheredData[ gatheredIndices[2*i] ];
    recCount = gatheredIndices[2*i+1] - gatheredIndices[2*i] + 1;

    // syntax
    /*MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag,
      MPI_Comm comm, MPI_Status *status)*/
    MPI_Irecv(recStart, recCount, MPI_INT, i, i, MPI_COMM_WORLD, &recReqs[i]);
  }
  

  MPI_Waitall(4, sendReqs, sendStats);
  MPI_Waitall(4, recReqs , recStats );
  MPI_Barrier(MPI_COMM_WORLD);
  //printf("Processor %d finished sending and receiveing!!!\n" , rank);

    
  /* do a local sort */
  //qsort(gatheredData, gatheredDataSize , sizeof(int), compare);
  
  // print the data at some node (TEST)
  if ( rank == 2 ) {
    printf("Data at %d:\n", rank);
    for ( j = 0 ; j <nTasks ; j++) {
      int start = indices[2*j];
      int end   = indices[2*j+1];
      for ( i = start ; i <= end ; i++ ){
	printf("From %d: gatheredData[%d] = %d\n" ,j, i,  gatheredData[i]);
      }
    }
  }





  /* every processor writes its result to a file */

  free(locData);
  MPI_Finalize();
  return 0;
}






























static int compare(const void *a, const void *b) {
  /* 
     return 1 if the value at *a is bigger
     than the value at *b
  */

  // cast to  pointers to int
  int *da = (int *)a;
  int *db = (int *)b;

  // if a > b return 1, ow return stuff
  if (*da > *db)
    return 1;
  else if (*da < *db)
    return -1;
  else
    return 0;
}
