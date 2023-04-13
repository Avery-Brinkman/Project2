#include <format>

#include "user.h"
#include <iostream>

using namespace USER_NS;

User::User(const std::string_view userName, const SOCKET& userSocket)
    : name(userName.data(), userName.size()), socket(userSocket) {}

void User::quit() {
  // Extra cleanup if necessary
  while (!m_groups.empty())
    leaveGroup(m_groups.begin()->first);

  // Close connection
  closesocket(socket);

  // Set quit to true so that we know why the socket isn't working when we try and use it later
  m_quit = true;
}

std::vector<int> User::joinedGroups() const {
  std::vector<int> ids;
  for (auto const& [id, group] : m_groups)
    ids.push_back(id);
  return ids;
}

void User::joinGroup(int groupId, std::shared_ptr<GROUP_NS::Group> group) {
  // Notify if already joined, else add to group
  if (m_groups.contains(groupId))
    sendMessage(std::format("Already in group {}!\n", groupId));
  else {
    // Add to list of joined groups
    m_groups[groupId] = group;
    // Tell group to add this user
    group->addUser(name);
    // Confirmation message
    sendMessage(std::format("Joined group {}!\n", groupId));
  }

  // Send the list of other group member names
  showGroupMembers(groupId);
}

void User::leaveGroup(int groupId) {
  // Can't leave unless already joined
  if (!verifyGroup(groupId))
    return;

  // Tell group to remove user
  m_groups.at(groupId)->removeUser(name);
  // Remove group from list of joined groups
  m_groups.erase(groupId);
  // Confirmation message
  sendMessage(std::format("Left group {}.\n", groupId));
}

void User::notifyJoin(const std::string_view userName, int groupId) {
  sendMessage(std::format("User {} has joined group {}\n", userName, groupId));
}

void User::notifyLeave(const std::string_view userName, int groupId) {
  sendMessage(std::format("User {} has left group {}\n", userName, groupId));
}

void User::showGroupMembers(int groupId) {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return;

  std::string memberListMsg = std::format("Group {} members:", groupId);

  // Add each name to the output message
  for (auto const& user : m_groups.at(groupId)->getUsers())
    memberListMsg.append(" " + user + ",");
  // Replace trailing comma with \n
  memberListMsg.back() = '\n';

  // Send the list of group members
  sendMessage(memberListMsg);
}

int User::postMessage(int groupId, const std::string_view subject, const std::string_view content) {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return -1;

  // Get id of the new message
  int msgId = m_groups.at(groupId)->postMessage(name, subject, content);
  // Send success message
  sendMessage(std::format("Message posted with id {}\n", msgId));

  // Return message id
  return msgId;
}

void User::getMessage(int groupId, int messageId) {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return;

  // Get the message
  auto message = m_groups.at(groupId)->getMessage(messageId);
  // Send response
  if (message.id == -1) {
    sendMessage(std::format("Group {} does not contain message {}!\n", groupId, messageId));
    return;
  }
  sendMessage(message.message);
}

void User::notifyMessage(int groupId, int messageId) {
  // Get the new message
  auto message = m_groups.at(groupId)->getMessage(messageId);

  // Send the info for the new message
  sendMessage(std::format("{}\n{}\n{}\n{}\n{}\n", groupId, message.id, message.userName,
                          message.postDate, message.subject));
}

int User::showLastMessages(int groupId) {
  // Make sure user belongs to the group before showing last messages
  if (!verifyGroup(groupId))
    return;

  // Get the ids of the last messages sent
  auto lastMessages = m_groups.at(groupId)->getLastMessages(2);
  // Send the number of messages about to be sent (so that client can make appropriate number of
  // reads)
  sendMessage(std::format("{}\n", lastMessages.size()));
  // Send notification for each of the messages
  for (auto id : lastMessages)
    notifyMessage(groupId, id);
}

bool User::verifyGroup(int groupId) {
  if (!m_groups.contains(groupId)) {
    sendMessage(std::format("You must join group {} before performing that action!\n", groupId));
    return false;
  }
  return true;
}

void User::invalidCommand(const std::string_view badCommand) {
  std::string notification = std::format("Invalid command sent: {}", badCommand);
  if (notification.back() != '\n')
    notification.append("\n");
  sendMessage(notification);
}

void User::sendMessage(const std::string_view message) {
  int res = send(socket, message.data(), (int)message.length(), 0);
  if (res == SOCKET_ERROR) {
    std::cout << "Failed to send message to " << name << ". Error: " << WSAGetLastError()
              << std::endl;
    closesocket(socket);
  }
}

void User::addCommand(std::string_view command) {
  m_commandQueue.push(command.data());
  m_commandSem.release();
}

std::string User::getNextCommand() {
  m_commandSem.acquire();
  std::string command = std::move(m_commandQueue.front());
  m_commandQueue.pop();
  return command;
}
