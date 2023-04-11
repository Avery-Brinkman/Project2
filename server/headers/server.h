#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <thread>
#include <vector>

#include "group.h"

constexpr auto DEFAULT_BUFLEN = 512;

namespace SERVER_NS {
class Server {
public:
  // Creates a new user given a socket for new client connection
  void addUser(const SOCKET& userSocket);

  // Starts a new thread to handle socket data, mapping it to the user it belongs to
  void addUser(const std::string_view userName, const SOCKET& userSocket);

  // Shuts down the server and all client connections
  void shutdown();

private:
  // Threaded function that handles the client connection
  void userHandler(std::stop_token stopToken, const std::string_view userName,
                   const SOCKET& userSocket);

  // The list of groups that run on the server. Indexed by ID, with 0 being the public (default)
  // group
  std::vector<GROUP_NS::Group> m_groups = std::vector<GROUP_NS::Group>(6);

  // Used to create stop_tokens, which allow for cooperative cancellation
  std::stop_source m_stopSource = std::stop_source();

  // Used to keep track of running threads
  std::map<std::string, std::jthread> m_users;
};
} // namespace SERVER_NS