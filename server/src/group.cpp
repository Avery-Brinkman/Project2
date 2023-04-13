#include <chrono>
#include <format>

#include "group.h"

using namespace GROUP_NS;

void Group::addUser(const std::string_view userName) {
  m_mutex.lock();
  m_users.emplace_back(userName.data());
  m_mutex.unlock();
}

void Group::removeUser(const std::string_view userName) {
  m_mutex.lock();
  for (auto user = m_users.begin(); user != m_users.end(); user++)
    if (*user == userName) {
      m_users.erase(user);
      m_mutex.unlock();
      return;
    }
  m_mutex.unlock();
}

int Group::postMessage(const std::string_view name, const std::string_view subject,
                       const std::string_view content) {
  m_mutex.lock();
  auto msgId = (int)m_messages.size();
  m_messages.emplace_back(
      msgId, name.data(), subject.data(), content.data(),
      std::format("{0:%D %R}", std::chrono::utc_clock::from_sys(std::chrono::system_clock::now())));
  m_mutex.unlock();
  return msgId;
}

Message Group::getMessage(const int messageId) {
  Message retMsg = {};
  m_mutex.lock();
  if (messageId >= 0 && messageId < m_messages.size())
    retMsg = m_messages.at(messageId);
  m_mutex.unlock();
  return retMsg;
}

std::vector<int> Group::getLastMessages(int n) {
  std::vector<int> ids;
  m_mutex.lock();
  for (auto iter = m_messages.end() - std::min((int)m_messages.size(), n); iter != m_messages.end();
       iter++)
    ids.push_back(iter->id);
  m_mutex.unlock();
  return ids;
}

std::vector<std::string> Group::getUsers() { return m_users; }
