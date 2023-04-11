#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>

#include "server.h"
#include <iostream>

int main() {
  try {
    auto s = SERVER_NS::Server::Server();
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
