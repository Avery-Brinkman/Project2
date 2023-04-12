#include <iostream>
#include <sstream>

#include "server.h"

using namespace SERVER_NS;

void Server::addUser(const SOCKET& userSocket) {
  // Read the userName into a buffer
  char nameBuf[DEFAULT_BUFLEN] = { '\0' };
  int res = recv(userSocket, nameBuf, DEFAULT_BUFLEN, 0);
  if (res <= 0) {
    std::cout << "Could not read from new user socket!" << std::endl;
    closesocket(userSocket);
    return;
  }

  // Read buffer into the string_view (exclude newline character), and pass on for thread creation
  //
  // Temporary fix until better end of command is decided
  //
  if (nameBuf[res - 1] == '\n')
    nameBuf[res - 1] = '\0';
  addUser(nameBuf, userSocket);
}

void Server::addUser(const std::string_view userName, const SOCKET& userSocket) {
  std::cout << "Attempting to add user: " << userName.data() << std::endl;
  // Ensure user doesn't already exist
  if (m_users.contains(userName.data())) {
    send(userSocket, "User already exists!", 21, 0);
    closesocket(userSocket);
    return;
  }

  // Create a new user, and hand it off to the thread function
  auto newUser = std::make_shared<USER_NS::User>(userName, userSocket, NUM_GROUPS);
  std::jthread(&Server::userHandler, this, newUser).detach();
  // Track the new user
  m_users[userName.data()] = newUser;
  std::cout << "Added user: " << userName.data() << std::endl;

  // send a list of users to group each time a new user joins 

}

void Server::shutdown() {
  // For each thread, request a stop and remove it from the map
}

void Server::userHandler(std::shared_ptr<USER_NS::User> user) {
  int res;

  // Receive until the user shuts down the connection or a stop is requested
  do {
    int sendRes;

    // Read data to buffer
    char recvbuf[DEFAULT_BUFLEN] = { '\0' };
    res = recv(user->socket, recvbuf, DEFAULT_BUFLEN, 0);

    // Close on error or if connection closed
    if (res <= 0) {
      if (res < 0)
        std::cout << "Could not read data from " << user->name << ", closing connection." << std::endl;
      else
        std::cout << "Connection to " << user->name << " closing..." << std::endl;
    }
    // Main logic
    else {

      parser(user, recvbuf);
    }
    //} while (res > 0 && !stopToken.stop_requested());
  } while (res > 0);

  // Close connection and remove from user map
  closesocket(user->socket);
  m_users.erase(user->name);
  return;
}

void Server::parser(std::shared_ptr<USER_NS::User> user, char* buffer) {
  if (strlen(buffer) < 6) {
    return;
  }
  if (buffer[0] != '%') {
    return;
  }
  std::string command(buffer, 5);
  int groupId = atoi(buffer + 6);
  std::cout << "   USER: " << user->name << std::endl;
  std::cout << "  GROUP: " << groupId << std::endl;
  std::cout << "COMMAND: ";
  if (command == "%quit") {
    std::cout << "    quit" << std::endl;
    quit(user);
    return;
  }
  else if (command == "%join") {
    std::cout << "join" << std::endl;
    addToGroup(user, groupId);
  }
  else if (command == "%exit") {
    std::cout << "exit" << std::endl;
  }
  else if (command == "%usrs") {
    std::cout << "usrs" << std::endl;
  }
  else if (command == "%post") {
    std::cout << "post" << std::endl;
  }
  else if (command == "%mesg") {
    std::cout << "mesg" << std::endl;
  }
  else if (command == "%grps") {
    std::cout << "grps" << std::endl;
  }
  else {
    std::cout << "Invalid command" << std::endl;
  }
  std::cout << std::endl;

  std::stringstream ss;
  ss << user->name << " " << command << " " << groupId << '\n';
  std::string ret = ss.str();
  send(user->socket, ret.c_str(), (int)ret.size(), 0);
}

void Server::quit(std::shared_ptr<USER_NS::User> user) {
  // Get the ids of groups that the user had joined
  auto joinedGroups = user->joinedGroups();

  // Close connection
  user->quit();

  // Stop tracking name
  m_users.erase(user->name);
  
  // Notify other users that the user has left the group
  for (auto groupId : joinedGroups) {
    for (auto otherUserName : m_groups[groupId]->getUsers())
      std::cout << "Notify leave" << std::endl;
  }
}

void Server::addToGroup(std::shared_ptr<USER_NS::User> user, int groupId) {
  // Get users currently in the group to notify them later
  auto existingUsers = m_groups[groupId]->getUsers();
  // Tell user to join the group
  user->joinGroup(groupId, m_groups[groupId]);
  // Notify other users
  for (auto userName : existingUsers) {
    m_users.at(userName)->notifyJoin(user->name, groupId);
  }
}