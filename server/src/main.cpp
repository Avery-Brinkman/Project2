#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <iostream>
#include <string>

#include "server.h"

constexpr auto DEFAULT_PORT = "5000";

int main(int argc, char* argv[]) {
  WSADATA wsaData;
  SERVER_NS::Server server;

  // Initialize Winsock
  int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0)
    throw std::exception("WSAStartup failed");

  addrinfo hints;
  addrinfo* result;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  const char* port = (argc >= 2) ? argv[1] : DEFAULT_PORT;
  res = getaddrinfo(nullptr, port, &hints, &result);
  if (res != 0) {
    WSACleanup();
    throw std::exception("getaddrinfo failed");
  }

  // Create a SOCKET for the server to listen for client connections.
  auto listenSocket = INVALID_SOCKET;
  listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listenSocket == INVALID_SOCKET) {
    freeaddrinfo(result);
    WSACleanup();
    throw std::exception("socket failed");
  }

  // Setup the TCP listening socket
  res = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (res == SOCKET_ERROR) {
    freeaddrinfo(result);
    closesocket(listenSocket);
    WSACleanup();
    throw std::exception("bind failed");
  }
  freeaddrinfo(result);

  res = listen(listenSocket, SOMAXCONN);
  if (res == SOCKET_ERROR) {
    closesocket(listenSocket);
    WSACleanup();
    throw std::exception("listen failed");
  }

  // Always listen for new user
  while (true) {
    auto userSocket = INVALID_SOCKET;
    // Accept a client socket
    userSocket = accept(listenSocket, nullptr, nullptr);
    if (userSocket == INVALID_SOCKET) {
      closesocket(listenSocket);
      WSACleanup();
      throw std::exception("accept failed");
    }

    server.addUser(userSocket);
  }

  server.shutdown();

  WSACleanup();
  return 0;
}
