#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>

#include "server.h"
#include <iostream>

int main() {
  auto s = SERVER_NS::Server::Server();

  return 0;
}
