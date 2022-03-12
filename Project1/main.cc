#include "game.h"
#include "utilities.h"
// Standard Includes for MPI, C and OS calls
#include <mpi.h>

// C++ standard I/O and library includes
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

// C++ stadard library using statements
using std::cout ;
using std::cerr ;
using std::endl ;

using std::vector ;
using std::string ;

using std::ofstream ;
using std::ifstream ;
using std::ios ;

// TRY:
// Vector<int> A(10);
// Recv(&A[0], 10, MPI_INT)

// MPI_Recv(&A[0], sizeof(move)*10, MPI_BYTE)

// MPI_Pack
// MPI_Unpack

void Server(int argc, char *argv[], int numProcessors) {

  // Check to make sure the server can run
  if(argc != 3) {
    cerr << "two arguments please!" << endl ;
    MPI_Abort(MPI_COMM_WORLD,-1) ;
  }

  // Input case filename 
  ifstream input(argv[1],ios::in) ;

  // Output case filename
  ofstream output(argv[2],ios::out) ;
  
  int count = 0 ;
  int NUM_GAMES = 0 ;
  // get the number of games from the input file
  input >> NUM_GAMES ;
  MPI_Request send_requests[NUM_GAMES];
  MPI_Request recv_requests[NUM_GAMES];
  unsigned char send_bufs[NUM_GAMES][IDIM*JDIM];
  move recv_bufs[NUM_GAMES][IDIM*JDIM];
  // game_state game_boards[NUM_GAMES];
  for(int i=0;i<NUM_GAMES;i+=numProcessors-1) {  // for each game in file...
    for(int j=1;j<numProcessors&&j+i-1<NUM_GAMES;++j) { // batch to each processor
      string input_string ;
      input >> input_string ;

      if(input_string.size() != IDIM*JDIM) {
        cerr << "something wrong in input file format!" << endl ;
        MPI_Abort(MPI_COMM_WORLD,-1) ;
      }
      // read in the initial game state from file
      for(int k=0;k<IDIM*JDIM;++k)
        send_bufs[i+j-1][k] = input_string[k] ;

      // Here we search for the solution to the game.   This is where most of
      // the work is performed.  We will want to farm these tasks out to other
      // processors in the parallel version.  To do this, send buf[] to the
      // client processor and use game_board.Init() on the Client to initialize
      // game board.  The result that will need to be sent back will either be
      // the moves required to solve the game or an indication that the game was
      // not solvable.
      MPI_Isend(send_bufs[i+j-1], IDIM*JDIM, MPI_UNSIGNED_CHAR, j, i+j, MPI_COMM_WORLD, &send_requests[i+j-1]);
      MPI_Irecv(recv_bufs[i+j-1], sizeof(move)*IDIM*JDIM, MPI_BYTE, MPI_ANY_SOURCE, i+j, MPI_COMM_WORLD, &recv_requests[i+j-1]);
    }
  }

  for(int i=0;i<NUM_GAMES;++i) { // recv all solutions
    MPI_Status status;
    int num;
    
    // MPI_Wait(&send_requests[i], MPI_STATUS_IGNORE);    
    MPI_Wait(&recv_requests[i], &status);
    MPI_Get_count(&status, MPI_BYTE, &num);

    // If the solution is found we want to output how to solve the puzzle
    // in the results file.
    if(num != 1) {
      num = num / sizeof(move);
      output << "found solution = " << endl;
      game_state s;
      s.Init(send_bufs[status.MPI_TAG-1]);
      s.Print(output) ;
      for(int j=0;j<num;++j) {
        if (recv_bufs[i][j].dir == -1)
          break;
        s.makeMove(recv_bufs[i][j]);
        output << "-->" << endl; 
        s.Print(output) ;
      }
      output << "solved" << endl ;
      count++ ;
    }
  }

  for(int i=1;i<numProcessors;++i) { //Send a close to all clients
    unsigned char buf[1];
    MPI_Isend(buf, 1, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD, &send_requests[i]);
  }

  // Report how cases had a solution.
  cout << "found " << count << " solutions" << endl ;

}

// Put the code for the client here
void Client() {
  bool con = true;
  do {
    unsigned char buf[IDIM*JDIM] ;
    MPI_Status status;
    MPI_Recv(buf, IDIM*JDIM, MPI_UNSIGNED_CHAR, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);  
    if (status.MPI_TAG == 0)
      con = false;
    else {
      // initialize the game
      game_state game_board ;
      game_board.Init(buf) ;

      // If we find a solution to the game, put the results in
      // solution
      move solution[IDIM*JDIM] ;
      int size = 0 ;
      // Search for a solution to the puzzle
      bool found = depthFirstSearch(game_board,size,solution) ;

      if (found) {
        MPI_Send(solution, sizeof(move)*size, MPI_BYTE, 0, status.MPI_TAG, MPI_COMM_WORLD); 
      }
      else {
        int send_size[1];
        MPI_Send(send_size, 1, MPI_BYTE, 0, status.MPI_TAG, MPI_COMM_WORLD);
      }
    }
  } while (con);
}


int main(int argc, char *argv[]) {
  // This is a utility routine that installs an alarm to kill off this
  // process if it runs to long.  This will prevent jobs from hanging
  // on the queue keeping others from getting their work done.
  chopsigs_() ;
  
  // All MPI programs must call this function
  MPI_Init(&argc,&argv) ;

  
  int myId ;
  int numProcessors ;

  /* Get the number of processors and my processor identification */
  MPI_Comm_size(MPI_COMM_WORLD,&numProcessors) ;
  MPI_Comm_rank(MPI_COMM_WORLD,&myId) ;

  if(myId == 0) {
    // Processor 0 runs the server code
    get_timer() ;// zero the timer
    Server(argc,argv, numProcessors) ;
    // Measure the running time of the server
    cout << "execution time = " << get_timer() << " seconds." << endl ;
  } else {
    // all other processors run the client code.
    Client() ;
  }

  // All MPI programs must call this before exiting
  MPI_Finalize() ;
}
