#include <thread>

#include "server.h"

using namespace SERVER_NS;

Server::Server() {
  WSADATA wsaData;

  // Initialize Winsock
  int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0) {
    throw std::exception("WSAStartup failed");
  }

  addrinfo hints;
  addrinfo* result;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  // Resolve the server address and port
  res = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);
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

  auto ClientSocket = INVALID_SOCKET;
  // Accept a client socket
  ClientSocket = accept(m_listenSocket, nullptr, nullptr);
  if (ClientSocket == INVALID_SOCKET) {
    closesocket(m_listenSocket);
    WSACleanup();
    throw std::exception("accept failed");
  }

  // No longer need server socket
  closesocket(m_listenSocket);

  char recvbuf[DEFAULT_BUFLEN];
  int recvbuflen = DEFAULT_BUFLEN;
  // Receive until the peer shuts down the connection
  do {
    int sendRes;

    res = recv(ClientSocket, recvbuf, recvbuflen, 0);
    if (res > 0) {
      printf("Bytes received: %d\n", res);

      // Echo the buffer back to the sender
      sendRes = send(ClientSocket, recvbuf, res, 0);
      if (sendRes == SOCKET_ERROR) {
        closesocket(ClientSocket);
        WSACleanup();
        throw std::exception("send failed");
      }
      printf("Bytes sent: %d\n", sendRes);
    } else if (res == 0)
      printf("Connection closing...\n");
    else {
      closesocket(ClientSocket);
      WSACleanup();
      throw std::exception("recv failed");
    }

  } while (res > 0);

  // shutdown the connection since we're done
  res = shutdown(ClientSocket, SD_SEND);
  if (res == SOCKET_ERROR) {
    closesocket(ClientSocket);
    WSACleanup();
    throw std::exception("shutdown failed");
  }

  // cleanup
  closesocket(ClientSocket);
  WSACleanup();
}
