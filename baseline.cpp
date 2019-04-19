#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

//run compiled code (for 5 philosophers) with mpirun -n 5 program

using namespace std;

//this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 10; 

//if you change this base, update the Makefile "clean" accordingly
const string fileBase = "outFile"; 

char READY = true;

int main ( int argc, char *argv[] ) 
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  int tag = 1;

  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of processes.
  p = MPI::COMM_WORLD.Get_size ( );

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank ( );
  
  //Safety check - need at least 2 philosophers to make sense
  if (p < 2) {
	    MPI::Finalize ( );
	    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
	    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;
  
  //setup message storage locations
//   int leftNeighbor = (id + p - 1) % p;
  int rightNeighbor = (id + 1) % p;
  
  std::string title;
  bool oddAmount = p % 2 != 0;
  if(p - 1 == id && oddAmount) {
      title = "last";
  } else if ( id % 2 == 0 ) {
      title = "even";
  } else {
      title = "odd";
  }




  pomerize P;

  string lFile = fileBase + to_string(id);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );
  bool firstIter = true;

  while (numWritten < MAXMESSAGES) {
    // Recieving
    if(!firstIter && title == "even"){
        MPI::COMM_WORLD.Recv( &READY, 1, MPI::BYTE, (p - 1), tag, status );
    } else if ( title == "odd") {
        MPI::COMM_WORLD.Recv( &READY, 1, MPI::BYTE, (id - 1), tag, status );
    } else if (title == "last") {
        for(int i = 1 ; i < p - 1; i += 2){
            MPI::COMM_WORLD.Recv( &READY, 1, MPI::BYTE, i, tag, status );
        }
    }

    if(title == "last") {
        std::cout << "LAST IN THIS MF" << std::endl;
    } else if (title == "even") {
        std::cout << "EVEN AF" << std::endl;
    } else {
        std::cout << "ODD BICH" << std::endl;
    }


    // Writing blocc
    foutLeft << id << "'s poem:" << endl;
    foutRight << id << "'s poem:" << endl;
    
    string stanza1, stanza2, stanza3;
    stanza1 = P.getLine();
    foutLeft << stanza1 << endl;
    foutRight << stanza1 << endl;

    stanza2 = P.getLine();
    foutLeft << stanza2 << endl;
    foutRight << stanza2 << endl;

    stanza3 = P.getLine();
    foutLeft << stanza3 << endl << endl;
    foutRight << stanza3 << endl << endl;

    numWritten++;
    // Writing blocc over


    if(title == "even"){
        MPI::COMM_WORLD.Send( &READY, 1, MPI::BYTE, (id + 1) % p, tag );
    } else if ( title == "odd" ) {
        MPI::COMM_WORLD.Send( &READY, 1, MPI::BYTE, (p - 1) , tag );
    } else if (title == "last" ) {
        for(int i = 0; i < p - 1; i += 2) {
            MPI::COMM_WORLD.Send( &READY, 1, MPI::BYTE, i , tag );
        }
    }

    firstIter = false;
  }

  foutLeft.close();
  foutRight.close();
  
  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}
