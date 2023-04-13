#include <format>
#include <iostream>

#include "server.h"

using namespace SERVER_NS;

Server::Server(int privateGroupCount) {
  for (int groupId = 0; groupId <= privateGroupCount; groupId++)
    m_groups.push_back(std::make_shared<GROUP_NS::Group>());
}

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
    send(userSocket, "User already exists!\n", 21, 0);
    closesocket(userSocket);
    return;
  }

  // Create a new user, and hand it off to the thread function
  auto newUser = std::make_shared<USER_NS::User>(userName, userSocket);
  std::jthread(&Server::userHandler, this, newUser).detach();
  // Track the new user
  m_users[userName.data()] = newUser;
  std::cout << "Added user: " << userName.data() << std::endl;
}

void Server::shutdown() {
  // For each thread, request a stop and remove it from the map
}

void Server::userHandler(std::shared_ptr<USER_NS::User> user) {
  int res;

  // Receive until the user shuts down the connection or a stop is requested
  do {
    // Read data to buffer
    char recvbuf[DEFAULT_BUFLEN] = {'\0'};
    res = recv(user->socket, recvbuf, DEFAULT_BUFLEN, 0);

    // Close on error or if connection closed
    if (res <= 0) {
      if (res < 0 && !user->selfQuit())
        std::cout << "Could not read data from " << user->name << ", closing connection."
                  << std::endl;
      else
        std::cout << "Connection to " << user->name << " closing..." << std::endl;
    }
    // Main logic
    else
      parser(user, recvbuf);
  } while (res > 0);

  // Remove user
  quit(user);
  return;
}

void Server::parser(std::shared_ptr<USER_NS::User> user, const char* buffer) {
  if (strlen(buffer) < 6 || buffer[0] != '%') {
    logCommand(user->name, "INVALID", -1);
    invalidCommand(user, buffer);
    return;
  }

  std::string command(buffer, 5);
  int groupId = atoi(buffer + 6);

  if (command == "%quit") {
    logCommand(user->name, "quit");
    quit(user);
  } else if (command == "%join") {
    logCommand(user->name, "join", groupId);
    addToGroup(user, groupId);
  } else if (command == "%exit") {
    logCommand(user->name, "exit", groupId);
    removeFromGroup(user, groupId);
  } else if (command == "%usrs") {
    logCommand(user->name, "usrs", groupId);
    showGroupMembers(user, groupId);
  } else if (command == "%post") {
    logCommand(user->name, "post", groupId);
    postMessage(user, groupId);
  } else if (command == "%mesg") {
    logCommand(user->name, "mesg", groupId);
    getMessage(user, groupId);
  } else if (command == "%grps") {
    logCommand(user->name, "grps");
    listGroups(user);
  } else {
    logCommand(user->name, "INVALID", -1);
    invalidCommand(user, command);
  }
}

void Server::quit(std::shared_ptr<USER_NS::User> user) {
  // Remove the user from each group that they joined
  for (auto groupId : user->joinedGroups())
    removeFromGroup(user, groupId);

  // Close connection
  user->quit();

  // Stop tracking name
  m_users.erase(user->name);
}

void Server::addToGroup(std::shared_ptr<USER_NS::User> user, int groupId) {
  // Get users currently in the group to notify them later
  auto existingUsers = m_groups[groupId]->getUsers();

  // Tell user to join the group
  user->joinGroup(groupId, m_groups[groupId]);

  // Notify other users
  for (auto const& userName : existingUsers)
    m_users.at(userName)->notifyJoin(user->name, groupId);
}

void Server::removeFromGroup(std::shared_ptr<USER_NS::User> user, int groupId) {
  // Tell user to leave group
  user->leaveGroup(groupId);

  // Notify other users
  for (auto const& userName : m_groups[groupId]->getUsers())
    m_users.at(userName)->notifyLeave(user->name, groupId);
}

void Server::showGroupMembers(std::shared_ptr<USER_NS::User> user, int groupId) const {
  // Tell user to show group members
  user->showGroupMembers(groupId);
}

void Server::listGroups(std::shared_ptr<USER_NS::User> user) const {
  std::string groupList;
  // Add each group id and member count to out message
  for (int groupId = 0; groupId < m_groups.size(); groupId++)
    groupList.append(
        std::format("Group {}: {} members", groupId, m_groups[groupId]->getUsers().size()));

  // Remove trailing space
  groupList.pop_back();
  // Replace trailing comma with \n
  groupList.back() = '\n';

  user->sendMessage(groupList);
}

void Server::postMessage(std::shared_ptr<USER_NS::User> user, int groupId) const {
  int res;

  // Read subject into buffer, and create a string_view for it
  char subjBuf[DEFAULT_BUFLEN] = {'\0'};
  res = recv(user->socket, subjBuf, DEFAULT_BUFLEN, 0);
  std::string_view subject(subjBuf, res);

  // Read content into buffer, and create a string_view for it
  char contBuf[DEFAULT_BUFLEN] = {'\0'};
  res = recv(user->socket, contBuf, DEFAULT_BUFLEN, 0);
  std::string_view content(contBuf, res);

  // Tell user to post the message and get the id for it
  int messageId = user->postMessage(groupId, subject, content);
  // Don't continue if the message couldn't be posted
  if (messageId < 0)
    return;

  // Notify the rest of the users
  for (auto const& userName : m_groups[groupId]->getUsers())
    m_users.at(userName)->notifyMessage(groupId, messageId);
}

void Server::getMessage(std::shared_ptr<USER_NS::User> user, int groupId) const {
  // Get messageId
  char recvbuf[DEFAULT_BUFLEN] = {'\0'};
  int res = recv(user->socket, recvbuf, DEFAULT_BUFLEN, 0);
  int messageId = atoi(recvbuf);
  // Have user get message with same id
  user->getMessage(groupId, messageId);
}

void Server::invalidCommand(std::shared_ptr<USER_NS::User> user,
                            const std::string_view badCommand) const {
  // Show user the bad command it made
  user->invalidCommand(badCommand);
}

void Server::logCommand(const std::string_view userName, const std::string_view command,
                        int groupId) const {
  std::cout << "USERNAME: " << userName << std::endl;
  std::cout << "GROUP_ID: " << groupId << std::endl;
  std::cout << " COMMAND: " << command << std::endl;
  std::cout << std::endl;
}
