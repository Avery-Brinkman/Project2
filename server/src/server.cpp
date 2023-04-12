#include <iostream>
#include <sstream>

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

  // Read buffer into the string_view (exclude newline character), and pass on for thread creation
  nameBuf[res - 1] = '\0';
  addUser(nameBuf, userSocket);
}

void Server::addUser(const std::string_view userName, const SOCKET& userSocket) {
  // Ensure user doesn't already exist
  if (m_users.contains(userName.data())) {
    send(userSocket, "User already exists!", 21, 0);
    closesocket(userSocket);
    return;
  }
  // Create a new thread and add it to our map
  std::jthread(&Server::userHandler, this, m_stopSource.get_token(), userName, userSocket).detach();
  m_users.emplace(userName.data());
}

void Server::shutdown() {
  // For each thread, request a stop and remove it from the map
}

void Server::userHandler(std::stop_token stopToken, const std::string_view userName,
                         const SOCKET& userSocket) {
  int res;
  std::string name(userName.data(), userName.size());

  // Receive until the user shuts down the connection or a stop is requested
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
    }
    // Main logic
    else {

      parser(name, userSocket, recvbuf);
    }
  } while (res > 0 && !stopToken.stop_requested());

  // Close connection and remove from user map
  closesocket(userSocket);
  m_users.erase(name);
  return;
}

void Server::parser(const std::string_view userName, const SOCKET& userSocket, char* buffer) {
  if (strlen(buffer) < 6) {
    return;
  }
  if (buffer[0] != '%') {
    return;
  }
  std::string command(buffer, 5);
  int groupId = atoi(buffer + 6);
  std::cout << "   USER: " << userName.data() << std::endl;
  std::cout << "  GROUP: " << groupId << std::endl;
  std::cout << "COMMAND: ";
  if (command == "%quit") {
    std::cout << "    quit" << std::endl;
    closesocket(userSocket);
    m_users.erase(userName.data());
    return;
  } else if (command == "%join") {
    std::cout << "join" << std::endl;
  } else if (command == "%exit") {
    std::cout << "exit" << std::endl;
  } else if (command == "%usrs") {
    std::cout << "usrs" << std::endl;
  } else if (command == "%post") {
    std::cout << "post" << std::endl;
  } else if (command == "%mesg") {
    std::cout << "mesg" << std::endl;
  } else if (command == "%grps") {
    std::cout << "grps" << std::endl;
  } else {
    std::cout << "Invalid command" << std::endl;
  }
  std::cout << std::endl;

  std::stringstream ss;
  ss << userName << " " << command << " " << groupId << '\n';
  std::string ret = ss.str();
  send(userSocket, ret.c_str(), (int)ret.size(), 0);
}
