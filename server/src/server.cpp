#include <iostream>

#include "server.h"

using namespace SERVER_NS;

void Server::addUser(const SOCKET& userSocket) {
  // Read the userName into a buffer
  char nameBuf[DEFAULT_BUFLEN] = {'\0'};
  int res = recv(userSocket, nameBuf, DEFAULT_BUFLEN, 0);
  if (res <= 0) {
    std::cout << "Could not read from new user socket!" << std::endl;
    closesocket(userSocket);
    return;
  }

  // Read buffer into the string (excluding newline), and create a new thread for it
  // std::string_view(nameBuf, res - 1);
  nameBuf[res - 1] = '\0';
  addUser(std::string_view(nameBuf), userSocket);
}

void Server::addUser(const std::string_view userName, const SOCKET& userSocket) {
  std::cout << "ADD USER: " << userName.data() << std::endl;
  if (m_users.contains(userName.data())) {
    send(userSocket, "User already exists!", 21, 0);
    closesocket(userSocket);
    return;
  }
  std::jthread userThread(&Server::userHandler, this, m_stopSource.get_token(), userName.data(),
                          userSocket);
  m_users[userName.data()] = std::move(userThread);
}

void Server::shutdown() {
  for (auto& [userName, thread] : m_users) {
    thread.request_stop();
    m_users.erase(userName);
  }
}

void Server::userHandler(std::stop_token stopToken, const std::string_view userName,
                         const SOCKET& userSocket) const {
  int res;
  std::string name = userName.data();

  // Receive until the peer shuts down the connection
  do {
    int sendRes;

    // Read data to buffer
    char recvbuf[DEFAULT_BUFLEN] = {'\0'};
    res = recv(userSocket, recvbuf, DEFAULT_BUFLEN, 0);
    // Close on error or if connection closed
    if (res <= 0) {
      if (res < 0)
        std::cout << "Could not read data from " << name << ", closing connection." << std::endl;
      else
        std::cout << "Connection to " << name << " closing..." << std::endl;
    } else {
      // DEMO
      // Reply to <MSG> with '<userName>: <MSG>'
      std::string recvMsg(recvbuf, res - 1);
      std::string echoMsg(name + ": " + recvMsg + '\n');

      sendRes = send(userSocket, echoMsg.data(), (int)echoMsg.size() + 1, 0);
      // Close connection if send failed
      if (sendRes == SOCKET_ERROR) {
        std::cout << "Could not send data to " << name << std::endl;
        break;
      }
    }
  } while (res > 0 && !stopToken.stop_requested());

  closesocket(userSocket);
  return;
}
