#include "user.h"

using namespace USER_NS;

User::User(const std::string_view userName, const SOCKET& userSocket, const int groupCount) {
  name = std::string(userName.data(), userName.size());
  socket = userSocket;
}

void User::quit() {
  while (!m_groups.empty())
    m_groups.begin()->second->removeUser(name);

  closesocket(socket);
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
  // Show this user the other group members
  std::string returnMessage = "Group " + std::to_string(groupId) + " joined! Members:";
  if (existingUsers.empty()) returnMessage.append(" None ");
  else
    for (auto user : existingUsers) {
      returnMessage.append(" " + user + ",");
    }
  returnMessage.back() = '\n';
  int res = send(socket, returnMessage.c_str(), returnMessage.length(), 0);
}

void USER_NS::User::notifyJoin(std::string_view userName, int groupId) const {
  std::string notification = "User " + std::string(userName) + " has joined group " + std::to_string(groupId) + "\n";
  int res = send(socket, notification.c_str(), notification.length(), 0);
}