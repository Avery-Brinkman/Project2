#pragma once

#include <string>
#include <vector>

namespace GROUP_NS {
  struct Message {
    int id;
    std::string userName;
    std::string subject;
    std::string message;
  };

  class Group {
  public:
    Group() = default;

    // Adds a user with a given userName to the group
    void addUser(const std::string_view userName);

    // Removes a user with the given userName from the group
    void removeUser(const std::string_view userName);

    // Posts a message to the group and returns the id for the new message.
    int postMessage(const std::string_view name, const std::string_view subject, const std::string_view content);

    // Gets the Message with the given id
    Message getMessage(const int messageId) const;

    // Gets the list of users in the group
    std::vector<std::string> getUsers() const;

  private:
    // List of users
    std::vector<std::string> m_users;

    // All messages sent
    std::vector<Message> m_messages;
  };
}