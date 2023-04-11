#include <thread>

#include "server.h"
#include <iostream>

using namespace SERVER_NS;

Server::Server(const char* port) {
  WSADATA wsaData;

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
  res = getaddrinfo(nullptr, port, &hints, &result);
  if (res != 0) {
    WSACleanup();
    throw std::exception("getaddrinfo failed");
  }

  // Create a SOCKET for the server to listen for client connections.
  m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if (m_listenSocket == INVALID_SOCKET) {
    freeaddrinfo(result);
    WSACleanup();
    throw std::exception("socket failed");
  }

  // Setup the TCP listening socket
  res = bind(m_listenSocket, result->ai_addr, (int)result->ai_addrlen);
  if (res == SOCKET_ERROR) {
    freeaddrinfo(result);
    closesocket(m_listenSocket);
    WSACleanup();
    throw std::exception("bind failed");
  }
  freeaddrinfo(result);

  res = listen(m_listenSocket, SOMAXCONN);
  if (res == SOCKET_ERROR) {
    closesocket(m_listenSocket);
    WSACleanup();
    throw std::exception("listen failed");
  }

  // Always listen for new user
  while (true) {
    auto userSocket = INVALID_SOCKET;
    // Accept a client socket
    userSocket = accept(m_listenSocket, nullptr, nullptr);
    if (userSocket == INVALID_SOCKET) {
      closesocket(m_listenSocket);
      WSACleanup();
      throw std::exception("accept failed");
    }

    // Read the userName into a buffer
    char nameBuf[DEFAULT_BUFLEN] = {'\0'};
    res = recv(userSocket, nameBuf, DEFAULT_BUFLEN, 0);
    if (res <= 0) {
      closesocket(userSocket);
      continue;
    }

    // Read buffer into the string (excluding newline)
    std::string userName(nameBuf, res - 1);
    m_userSockets[userName] = userSocket;
    std::jthread(&Server::clientHandler, this, userName, userSocket).detach();
  }

  WSACleanup();
}

void Server::clientHandler(const std::string_view userName, const SOCKET& userSocket) const {
  int res;
  // Receive until the peer shuts down the connection
  do {
    int sendRes;

    // Read data to buffer
    char recvbuf[DEFAULT_BUFLEN] = {'\0'};
    res = recv(userSocket, recvbuf, DEFAULT_BUFLEN, 0);
    // Close on error or if connection closed
    if (res <= 0) {
      if (res < 0)
        std::cout << "Could not read data from " << userName.data() << ", closing connection."
                  << std::endl;
      else
        std::cout << "Connection to " << userName.data() << " closing..." << std::endl;
      closesocket(userSocket);
      return;
    }

    // DEMO
    // Reply to <MSG> with '<userName>: <MSG>'
    std::string recvMsg(recvbuf, res - 1);
    std::string echoMsg(std::string(userName.data()) + ": " + recvMsg + '\n');

    sendRes = send(userSocket, echoMsg.data(), (int)echoMsg.size() + 1, 0);
    // Close connection if send failed
    if (sendRes == SOCKET_ERROR) {
      std::cout << "Could not send data to " << userName.data() << std::endl;
      closesocket(userSocket);
      return;
    }
  } while (res > 0);
}
