#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <format>
#include <iostream>
#include <string>

#include "server.h"

constexpr auto DEFAULT_PORT = "5000";

int main(int argc, char* argv[]) {
  std::cout << "Starting server!" << std::endl;
  WSADATA wsaData;
  SERVER_NS::Server server;

  // Initialize Winsock
  int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0)
    throw std::exception(std::format("WSAStartup failed with error: {}", res).c_str());

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
    throw std::exception(std::format("getaddrinfo failed with error: {}", res).c_str());
  }

  // Create a SOCKET for the server to listen for client connections.
  auto listenSocket = INVALID_SOCKET;
  listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (listenSocket == INVALID_SOCKET) {
    freeaddrinfo(result);
    WSACleanup();
    throw std::exception(std::format("socket failed with error: {}", WSAGetLastError()).c_str());
  }

  // Setup the TCP listening socket
  res = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (res == SOCKET_ERROR) {
    freeaddrinfo(result);
    closesocket(listenSocket);
    WSACleanup();
    throw std::exception(std::format("bind failed with error: {}", WSAGetLastError()).c_str());
  }
  freeaddrinfo(result);

  res = listen(listenSocket, SOMAXCONN);
  if (res == SOCKET_ERROR) {
    closesocket(listenSocket);
    WSACleanup();
    throw std::exception(std::format("listen failed with error: {}", WSAGetLastError()).c_str());
  }

  std::cout << "Server running on port " << port << std::endl;
  std::cout << "Waiting for connections..." << std::endl;

  // Always listen for new user
  while (true) {
    auto userSocket = INVALID_SOCKET;
    // Accept a client socket
    userSocket = accept(listenSocket, nullptr, nullptr);
    std::cout << "Connection made with new device." << std::endl;
    if (userSocket == INVALID_SOCKET) {
      closesocket(listenSocket);
      WSACleanup();
      throw std::exception(std::format("accept failed with error: {}", WSAGetLastError()).c_str());
    }
    server.addUser(userSocket);
  }
  server.shutdown();

  WSACleanup();
  return 0;
}
