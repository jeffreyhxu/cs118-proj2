#include <iostream>
#include <string>

#include <stdlib.h>

using namespace std;

int main (int argc, char* argv[]) {
  if (argc != 4) {
    cout << "Invalid Number of Arguments" << endl;
    return 0;
  }

  string hostname = argv[1];
  short portnum = atoi(argv[2]);
  string filename = argv[3];

  //cerr << hostname << endl << portnum << endl << filename << endl;

}
