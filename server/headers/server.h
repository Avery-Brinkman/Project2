#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

#include "group.h"
#include "user.h"

constexpr auto DEFAULT_BUFLEN = 512;
constexpr auto NUM_GROUPS = 5;

namespace SERVER_NS {
class Server {
public:
  // Server object reads from users sockets and coordinates server actions
  explicit Server(int privateGroupCount = NUM_GROUPS);

  // Creates a new user thread given a socket for new client connection
  void addUser(const SOCKET& userSocket);

  // Shuts down the server and all client connections
  void shutdown();

private:
  // Gets and sets user's name and starts tracking the user in the server
  void addUserToServer(std::shared_ptr<USER_NS::User> user);

  // Reads characters from a socket up to and including \n, returning result
  std::string readSocket(const SOCKET& userSocket);

  // Reads characters from a socket up to but not including \n, and adds it to the queue
  void addToQueue(const SOCKET& userSocket, std::queue<std::string>& queue);

  // Constantly reads a user socket and adds the commands to the user command queue
  void readHandler(std::shared_ptr<USER_NS::User> user);

  // Threaded function that handles the client connection. This is the main logic that runs a user
  // connection
  void userHandler(std::shared_ptr<USER_NS::User> user);

  // Takes the user's input and, logs it, and calls the relevant function
  void parser(std::shared_ptr<USER_NS::User> user, const std::string_view command);

  // Removes the user from any joined group, tells user to close connection, and removes from list
  // of users
  void quit(std::shared_ptr<USER_NS::User> user);

  // Adds user to a group, notifying all other users that they joined
  void addToGroup(std::shared_ptr<USER_NS::User> user, int groupId);

  // Removes user from a group, notifying all other users that they left
  void removeFromGroup(std::shared_ptr<USER_NS::User> user, int groupId);

  // Has user send a list of members in a group
  void showGroupMembers(std::shared_ptr<USER_NS::User> user, int groupId) const;

  // Creates a message with each available group and its member count
  void listGroups(std::shared_ptr<USER_NS::User> user) const;

  // Reads message info, has user post it, notifying all other users
  void postMessage(std::shared_ptr<USER_NS::User> user, int groupId);

  // Reads message info and has user retrieve its contents
  void getMessage(std::shared_ptr<USER_NS::User> user, int groupId) const;

  // Tells user to send invalid command message
  void invalidCommand(std::shared_ptr<USER_NS::User> user, const std::string_view badCommand) const;

  // Logs the use of a command to the console
  void logCommand(const std::string_view userName, const std::string_view command,
                  int groupId = 0) const;

  // The list of groups that run on the server. Indexed by Id, with 0 being the public (default)
  // group
  std::vector<std::shared_ptr<GROUP_NS::Group>> m_groups;

  // Mutex to protect access to the users map
  std::mutex m_usrsMutex;

  // Used to keep track of user objects by name
  std::map<std::string, std::shared_ptr<USER_NS::User>> m_users;
};
} // namespace SERVER_NS
