#include "user.h"

using namespace USER_NS;

User::User(const std::string_view userName, const SOCKET& userSocket) {
  name = std::string(userName.data(), userName.size());
  socket = userSocket;
}

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
  for (auto [id, group] : m_groups)
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
  std::string returnMessage = "Joined group " + std::to_string(groupId) + "!\n";
  int res = send(socket, returnMessage.c_str(), returnMessage.length(), 0);

  // Send the list of other group member names
  showGroupMembers(groupId);

  // NEED TO SHOW LAST 2 MESSAGES AS WELL
}

void User::leaveGroup(int groupId) {
  // Tell group to remove user
  m_groups.at(groupId)->removeUser(name);
  // Remove group from list of joined groups
  m_groups.erase(groupId);
}

void User::notifyJoin(std::string_view userName, int groupId) const {
  std::string notification = "User " + std::string(userName) + " has joined group " + std::to_string(groupId) + "\n";
  int res = send(socket, notification.c_str(), notification.length(), 0);
}

void User::notifyLeave(std::string_view userName, int groupId) const {
  std::string notification = "User " + std::string(userName) + " has left group " + std::to_string(groupId) + "\n";
  int res = send(socket, notification.c_str(), notification.length(), 0);
}

void User::showGroupMembers(int groupId) const {
  int res;
  std::string returnMessage = "Group " + std::to_string(groupId) + " members:";

  // Make sure user belongs to the group before showing group members
  if (!m_groups.contains(groupId)) {
    returnMessage = "You must join group " + std::to_string(groupId) + " before seeing its members!\n";
    res = send(socket, returnMessage.c_str(), returnMessage.length(), 0);
    return;
  }

  // Get names of group members
  auto groupNames = m_groups.at(groupId)->getUsers();
  // Add each name to the output message
  for (auto user : groupNames)
    returnMessage.append(" " + user + ",");
  // Replace trailing comma with \n
  returnMessage.back() = '\n';

  // Send list of group members
  res = send(socket, returnMessage.c_str(), returnMessage.length(), 0);
}