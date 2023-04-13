#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace GROUP_NS {
struct Message {
  int id = -1;
  std::string userName;
  std::string subject;
  std::string message;
  std::string postDate;
};

class Group {
public:
  Group() = default;

  // Adds a user with a given userName to the group
  void addUser(const std::string_view userName);

  // Removes a user with the given userName from the group
  void removeUser(const std::string_view userName);

  // Posts a message to the group and returns the id for the new message.
  int postMessage(const std::string_view name, const std::string_view subject,
                  const std::string_view content);

  // Gets the Message with the given id
  Message getMessage(const int messageId);

  // Returns the last n message ids
  std::vector<int> getLastMessages(int n);

  // Gets the list of users in the group
  std::vector<std::string> getUsers();

private:
  std::mutex m_mutex;

  // List of users
  std::vector<std::string> m_users = {};

  // All messages sent indexed by id
  std::vector<Message> m_messages = {};
};
} // namespace GROUP_NS
