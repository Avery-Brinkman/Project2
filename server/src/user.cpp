#include <format>

#include "user.h"

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
  // Add to list of joined groups
  m_groups[groupId] = group;
  // Get users already in the group
  auto existingUsers = group->getUsers();
  // Tell group to add this user
  group->addUser(name);

  // Confirmation message
  std::string returnMessage = std::format("Joined group {}!\n", groupId);
  int res = send(socket, returnMessage.c_str(), (int)returnMessage.length(), 0);

  // Send the list of other group member names
  showGroupMembers(groupId);

  // NEED TO SHOW LAST 2 MESSAGES AS WELL
}

void User::leaveGroup(int groupId) {
  // Can't leave unless already joined
  if (!verifyGroup(groupId))
    return;

  // Tell group to remove user
  m_groups.at(groupId)->removeUser(name);
  // Remove group from list of joined groups
  m_groups.erase(groupId);
}

void User::notifyJoin(const std::string_view userName, int groupId) const {
  std::string notification = std::format("User {} has joined group {}\n", userName, groupId);
  int res = send(socket, notification.c_str(), (int)notification.length(), 0);
}

void User::notifyLeave(const std::string_view userName, int groupId) const {
  std::string notification = std::format("User {} has left group {}\n", userName, groupId);
  int res = send(socket, notification.c_str(), (int)notification.length(), 0);
}

void User::showGroupMembers(int groupId) const {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return;

  std::string returnMessage = std::format("Group {} members:", groupId);

  // Add each name to the output message
  for (auto const& user : m_groups.at(groupId)->getUsers())
    returnMessage.append(" " + user + ",");
  // Replace trailing comma with \n
  returnMessage.back() = '\n';

  // Send list of group members
  int res = send(socket, returnMessage.c_str(), (int)returnMessage.length(), 0);
}

int User::postMessage(int groupId, const std::string_view subject,
                      const std::string_view content) const {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return -1;

  // Return the id of the new message
  return m_groups.at(groupId)->postMessage(name, subject, content);
}

void User::getMessage(int groupId, int messageId) const {
  // Make sure user belongs to the group before showing group members
  if (!verifyGroup(groupId))
    return;

  int res;
  auto message = m_groups.at(groupId)->getMessage(messageId);
  if (message.id == -1) {

    std::string returnMessage =
        std::format("Group {} does not contain message {}!\n", groupId, messageId);
    send(socket, returnMessage.c_str(), (int)returnMessage.length(), 0);
    return;
  }
  res = send(socket, message.message.c_str(), (int)message.message.length(), 0);
}

void User::notifyMessage(int groupId, int messageId) const {
  auto message = m_groups.at(groupId)->getMessage(messageId);

  std::string notification = std::format("{}\n{}\n{}\n{}\n{}\n", groupId, message.id,
                                         message.userName, message.postDate, message.subject);
  int res = send(socket, notification.c_str(), (int)notification.length(), 0);
}

bool User::verifyGroup(int groupId) const {
  if (!m_groups.contains(groupId)) {
    std::string returnMessage =
        std::format("You must join group {} before performing that action!\n", groupId);
    send(socket, returnMessage.c_str(), (int)returnMessage.length(), 0);
    return false;
  }
  return true;
}

void User::invalidCommand(const std::string_view badCommand) const {
  std::string notification = "Invalid command sent: " + std::string(badCommand);
  if (notification.back() != '\n')
    notification.append("\n");
  int res = send(socket, notification.c_str(), (int)notification.length(), 0);
}
