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
  void addUser(const SOCKET& userSocket);

  void addUser(const std::string_view userName, const SOCKET& userSocket);

  void shutdown();

private:
  void userHandler(std::stop_token stopToken, const std::string_view userName,
                   const SOCKET& userSocket) const;

  std::vector<GROUP_NS::Group> m_groups = std::vector<GROUP_NS::Group>(6);

  std::stop_source m_stopSource = std::stop_source();

  // needed?
  std::map<std::string, std::jthread> m_users;
};
} // namespace SERVER_NS