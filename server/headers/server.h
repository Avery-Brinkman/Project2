#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <thread>
#include <vector>

#include "group.h"
#include "user.h"

constexpr auto DEFAULT_BUFLEN = 512;
constexpr auto NUM_GROUPS = 5;

namespace SERVER_NS {
  class Server {
  public:
    Server(int privateGroupCount = NUM_GROUPS);

    // Creates a new user given a socket for new client connection
    void addUser(const SOCKET& userSocket);

    // Starts a new thread to handle socket data, mapping it to the user it belongs to
    void addUser(const std::string_view userName, const SOCKET& userSocket);

    // Shuts down the server and all client connections
    void shutdown();

  private:
    // Threaded function that handles the client connection
    void userHandler(std::shared_ptr<USER_NS::User> user);
    //void userHandler(std::stop_token stopToken, USER_NS::User& user);

    void parser(std::shared_ptr<USER_NS::User> user, char* buffer);

    void quit(std::shared_ptr<USER_NS::User> user);

    void addToGroup(std::shared_ptr<USER_NS::User> user, int groupId);

    void removeFromGroup(std::shared_ptr<USER_NS::User> user, int groupId);

    void showGroupMembers(std::shared_ptr<USER_NS::User> user, int groupId) const;

    void listGroups(std::shared_ptr<USER_NS::User> user) const;

    void postMessage(std::shared_ptr<USER_NS::User> user, int groupId) const;

    void getMessage(std::shared_ptr<USER_NS::User> user, int groupId) const;

    void invalidCommand(std::shared_ptr<USER_NS::User> user, const std::string_view badCommand) const;

    void logCommand(const std::string_view userName, const std::string_view command, int groupId = 0) const;

    // Used to create stop_tokens, which allow for cooperative cancellation
    //std::stop_source m_stopSource = std::stop_source();

    // The list of groups that run on the server. Indexed by Id, with 0 being the public (default)
    // group
    std::vector<std::shared_ptr<GROUP_NS::Group>> m_groups;

    // Used to keep track of user objects by name
    std::map<std::string, std::shared_ptr<USER_NS::User>> m_users;
  };
} // namespace SERVER_NS