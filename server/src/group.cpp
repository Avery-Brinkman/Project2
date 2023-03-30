#include "group.h"

using namespace GROUP_NS;

void Group::addUser(const std::string_view userName) {
  m_users.push_back(userName.data());
}

void Group::removeUser(const std::string_view userName) {
  for (auto user = m_users.begin(); user != m_users.end(); user++) {
    if (*user == userName) {
      m_users.erase(user);
      return;
    }
  }
}


std::vector<std::string> Group::getUsers() const {
  return m_users;
}