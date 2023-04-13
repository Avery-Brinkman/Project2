#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "group.h"

namespace USER_NS {
class User {
public:
  // Creates a user with a given name and existing socket
  User(const std::string_view userName, const SOCKET& userSocket);

  // Leaves any group they're still in and closes the socket
  void quit();

  // Returns the ids of each group they've joined
  std::vector<int> joinedGroups() const;

  // Adds group to list of joined groups, shows group members, and last 2 messages
  void joinGroup(int groupId, std::shared_ptr<GROUP_NS::Group> group);

  // Removes user from each joined group, stops tracking group, and sends success message
  void leaveGroup(int groupId);

  // Creates a message showing a user has joined a group and sends it
  void notifyJoin(const std::string_view userName, int groupId) const;

  // Creates a message showing a user has left a group and sends it
  void notifyLeave(const std::string_view userName, int groupId) const;

  // Creates a message showing the list of group members and sends it
  void showGroupMembers(int groupId) const;

  // Adds a message to a group, sending a success response and returning the message id
  int postMessage(int groupId, const std::string_view subject,
                  const std::string_view content) const;

  // Gets the message with a given id from a given group and returns the contents
  void getMessage(int groupId, int messageId) const;

  // Creates a message showing a new message has been posted to a group and sends it
  void notifyMessage(int groupId, int messageId) const;

  // Creates a message showing the last messages sent (up to 2) and returns the number that was sent
  int showLastMessages(int groupId) const;

  bool selfQuit() const { return m_quit; }

  // Returns whether user is in a given group and sends a message if not
  bool verifyGroup(int groupId) const;

  // Notifies the user that a bad command was sent
  void invalidCommand(const std::string_view badCommand) const;

  // Sends a message to the user
  void sendMessage(const std::string_view message) const;

  // Name of the user
  std::string name;

  // Socket user is connected with
  SOCKET socket;

private:
  // Maps group id to group
  std::map<int, std::shared_ptr<GROUP_NS::Group>> m_groups;

  // Tracks if user closed its own socket (for when future socket uses by server fail)
  bool m_quit = false;
};
}; // namespace USER_NS
