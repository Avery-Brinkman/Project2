#include <format>
#include <iostream>
#include <queue>
#include <sstream>

#include "server.h"

using namespace SERVER_NS;

Server::Server(int privateGroupCount) {
  for (int groupId = 0; groupId <= privateGroupCount; groupId++)
    m_groups.push_back(std::make_shared<GROUP_NS::Group>());
}

std::string Server::readSocket(const SOCKET& userSocket) {
  char recvBuf[DEFAULT_BUFLEN] = {'\0'};
  int res = recv(userSocket, recvBuf, DEFAULT_BUFLEN, 0);
  if (res <= 0) {
    if (res < 0) {
      std::cout << "Could not read from user socket! Error: " << WSAGetLastError() << std::endl;
      closesocket(userSocket);
      return "ERROR\n";
    }
    return "CLOSE\n";
  }

  return std::string(recvBuf, res);
}

void Server::addToQueue(const SOCKET& userSocket, std::queue<std::string>& queue) {
  std::string recvBuf = readSocket(userSocket);

  std::stringstream readStream(recvBuf);
  std::string currentCommand;
  while (std::getline(readStream, currentCommand, '\n'))
    queue.push(currentCommand);
}

void Server::readHandler(std::shared_ptr<USER_NS::User> user) {
  std::string currentCommand = "";
  do {
    std::string recvBuf = readSocket(user->socket);
    std::stringstream readStream(recvBuf);
    while (std::getline(readStream, currentCommand, '\n')) {
      if (currentCommand == "ERROR" || currentCommand == "CLOSE") {
        quit(user);
        break;
      }
      user->addCommand(currentCommand);
    }
  } while (currentCommand != "ERROR" && currentCommand != "CLOSE");
}

void Server::addUser(const SOCKET& userSocket) {
  auto newUser = std::make_shared<USER_NS::User>(userSocket);
  std::jthread(&Server::userHandler, this, newUser).detach();
}

void Server::shutdown() {
  // For each thread, request a stop and remove it from the map
}

void Server::addUserToServer(std::shared_ptr<USER_NS::User> user) {
  // Read the userName into a commandQueue
  std::queue<std::string> commandQueue;
  addToQueue(user->socket, commandQueue);

  std::string userName = commandQueue.front();
  commandQueue.pop();
  user->name = userName;

  if (userName == "ERROR" || userName == "CLOSE")
    return;
  std::cout << "Attempting to add user: " << userName.data() << std::endl;
  m_usrsMutex.lock();
  // Ensure user doesn't already exist
  if (m_users.contains(userName.data())) {
    m_usrsMutex.unlock();
    std::string msg = std::format("User {} already exists!\n", userName);
    send(user->socket, msg.c_str(), msg.size(), 0);
    closesocket(user->socket);
    return;
  }

  while (!commandQueue.empty()) {
    user->addCommand(commandQueue.front());
    commandQueue.pop();
  }
  // Track the new user
  m_users[userName.data()] = user;
  m_usrsMutex.unlock();
  std::cout << "Added user: " << userName.data() << std::endl;
}

void Server::userHandler(std::shared_ptr<USER_NS::User> user) {
  addUserToServer(user);

  bool run = true;
  std::jthread readThread(&Server::readHandler, this, user);
  // Receive until the user shuts down the connection or a stop is requested
  do {
    std::string command = std::move(user->getNextCommand());
    // Close on error or if connection closed
    if (command == "ERROR" && !user->selfQuit()) {
      std::cout << "Could not read data from " << user->name << ", closing connection."
                << std::endl;
      run = false;
    } else if (command == "CLOSE") {
      std::cout << "Connection to " << user->name << " closing..." << std::endl;
      run = false;
    }
    // Main logic
    else
      parser(user, command);
  } while (run);

  // Remove user
  quit(user);
  readThread.join();
  return;
}

void Server::parser(std::shared_ptr<USER_NS::User> user, const std::string_view command) {
  if (command.size() < 5 || command.front() != '%') {
    logCommand(user->name, "INVALID", -1);
    invalidCommand(user, command);
    return;
  }

  std::string commandName(command.data(), 5);
  int groupId = atoi(command.data() + 6);

  if (commandName == "%quit") {
    logCommand(user->name, "quit");
    quit(user);
  } else if (commandName == "%join") {
    logCommand(user->name, "join", groupId);
    addToGroup(user, groupId);
  } else if (commandName == "%exit") {
    logCommand(user->name, "exit", groupId);
    removeFromGroup(user, groupId);
  } else if (commandName == "%usrs") {
    logCommand(user->name, "usrs", groupId);
    showGroupMembers(user, groupId);
  } else if (commandName == "%post") {
    logCommand(user->name, "post", groupId);
    postMessage(user, groupId);
  } else if (commandName == "%mesg") {
    logCommand(user->name, "mesg", groupId);
    getMessage(user, groupId);
  } else if (commandName == "%grps") {
    logCommand(user->name, "grps");
    listGroups(user);
  } else {
    logCommand(user->name, "INVALID", -1);
    invalidCommand(user, commandName);
  }
}

void Server::quit(std::shared_ptr<USER_NS::User> user) {
  // Remove the user from each group that they joined
  for (auto groupId : user->joinedGroups())
    removeFromGroup(user, groupId);
  m_usrsMutex.lock();

  // Close connection
  user->quit();

  // Stop tracking name
  m_users.erase(user->name);
  m_usrsMutex.unlock();
}

void Server::addToGroup(std::shared_ptr<USER_NS::User> user, int groupId) {
  m_usrsMutex.lock();
  // Get users currently in the group to notify them later
  auto existingUsers = m_groups[groupId]->getUsers();

  // Tell user to join the group
  user->joinGroup(groupId, m_groups[groupId]);

  // Notify other users
  for (auto const& userName : existingUsers)
    m_users.at(userName)->notifyJoin(user->name, groupId);
  m_usrsMutex.unlock();
}

void Server::removeFromGroup(std::shared_ptr<USER_NS::User> user, int groupId) {
  m_usrsMutex.lock();
  // Tell user to leave group
  user->leaveGroup(groupId);

  // Notify other users
  for (auto const& userName : m_groups[groupId]->getUsers())
    m_users.at(userName)->notifyLeave(user->name, groupId);
  m_usrsMutex.unlock();
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
        std::format("Group {}: {} members, ", groupId, m_groups[groupId]->getUsers().size()));

  // Remove trailing space
  groupList.pop_back();
  // Replace trailing comma with \n
  groupList.back() = '\n';

  user->sendMessage(groupList);
}

void Server::postMessage(std::shared_ptr<USER_NS::User> user, int groupId) {
  // Read subject
  std::string subject = user->getNextCommand();

  // Read content
  std::string content = user->getNextCommand();

  // Tell user to post the message and get the id for it
  int messageId = user->postMessage(groupId, subject, content);
  // Don't continue if the message couldn't be posted
  if (messageId < 0)
    return;

  m_usrsMutex.lock();
  // Notify the rest of the users
  for (auto const& userName : m_groups[groupId]->getUsers())
    m_users.at(userName)->notifyMessage(groupId, messageId);
  m_usrsMutex.unlock();
}

void Server::getMessage(std::shared_ptr<USER_NS::User> user, int groupId) const {
  // Get messageId
  int messageId = atoi(user->getNextCommand().c_str());
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
