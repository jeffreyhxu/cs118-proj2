#include "tcp_server.h"

#include <iostream>
#include <string>

#include <stdlib.h>

using namespace std;

int main (int argc, char* argv[]) {
  if (argc != 2) {
    cout << "Invalid Number of Arguments" << endl;
    return 0;
  }

  unsigned short portnum = atoi(argv[1]);

  //cout << portnum << endl;

  TCP_server server = TCP_server(portnum);
  server.startServer();

}
