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

int Group::postMessage(const std::string_view name, const std::string_view subject, const std::string_view content) {
  int msgId = (int)m_messages.size();
  m_messages.emplace_back(msgId, name.data(), subject.data(), content.data());
  return msgId;
}

Message Group::getMessage(const int messageId) const {
  if (messageId < 0 || messageId >= m_messages.size()) return {};

  return m_messages.at(messageId);
}

std::vector<std::string> Group::getUsers() const {
  return m_users;
}