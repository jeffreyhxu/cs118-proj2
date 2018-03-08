#include "tcp_client.h"

#include <iostream>
#include <string>

#include <stdlib.h>

using namespace std;

int main (int argc, char* argv[]) {
  if (argc != 4) {
    cout << "Invalid Number of Arguments" << endl;
    return 0;
  }

  char* hostname = argv[1];
  unsigned short portnum = atoi(argv[2]);
  char* filename = argv[3];

  //cerr << hostname << endl << portnum << endl << filename << endl;

  TCP_client client = TCP_client(hostname, portnum);
  client.sendMessage();

}
