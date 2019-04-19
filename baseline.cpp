// Name: Joshua Pondrom, Mashroor Rashid
// Course: CS3800 "Introduction to Operating systems"
// Section: "The better one"
// Assignment: Dining Philosophers, Read-Write Problem
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

MPI::Status status;

void send(unsigned int to) {
    char READY = true;
    MPI::COMM_WORLD.Send( &READY, 1, MPI::BYTE, to, 1 );
}

void recv(unsigned int from) {
    char READY = true;
    MPI::COMM_WORLD.Recv( &READY, 1, MPI::BYTE, from, 1, status );
}

int main ( int argc, char *argv[] ) 
{

  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of processes.
  int p = MPI::COMM_WORLD.Get_size ( );

  //  Determine the rank of this process.
  int id = MPI::COMM_WORLD.Get_rank ( );
  
  //Safety check - need at least 2 philosophers to make sense
  if (p < 2) {
	    MPI::Finalize ( );
	    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
	    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;
  
  //setup message storage locations
  int left = (id + p - 1) % p;
  int right = (id + 1) % p;
  int last = (p - 1);
  
  //determine set position
  std::string title;
  bool oddAmount = p % 2 != 0;
  if(last == id && oddAmount) {
      title = "last";
  } else if ( id % 2 == 0 ) {
      title = "even";
  } else {
      title = "odd";
  }

  pomerize P;

  string lFile = fileBase + to_string(id);
  string rFile = fileBase + to_string(right);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );
  bool firstIter = true;

  while (numWritten < MAXMESSAGES) {
    // Recieving
    if(!firstIter && title == "even"){
        if(oddAmount){
            recv(last);
        } else {
            recv(right);
            recv(left);
        }
    } else if ( title == "odd") {
        if(oddAmount){
            recv(left);
            if( id != p - 2 ) {
                recv(right);
            }
        } else {
            recv(left);
            recv(right);
        }
    } else if (title == "last") {
        for(int i = 1 ; i < p - 1; i += 2){
            recv(i);
        }
    }

    // Writing block
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
    
    // Sending
    if(title == "even"){
        if(oddAmount){
            if(id != 0){
                send(left);
            }
            send(right);
        } else {
            send(left);
            send(right);
        }
    } else if ( title == "odd" ) {
        if(oddAmount){
            send(last);
        } else {
            send(left);
            send(right);
        }
    } else if (title == "last" ) {
        for(int i = 0; i < p - 1; i += 2) {
            send(i);
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
