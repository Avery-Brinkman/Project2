#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <vector>

#include "group.h"

constexpr auto DEFAULT_BUFLEN = 512;
constexpr auto DEFAULT_PORT = "5000";

namespace SERVER_NS {
class Server {
public:
  Server();

private:
  std::vector<GROUP_NS::Group> m_groups;

  SOCKET m_listenSocket = INVALID_SOCKET;

  std::map<std::string, SOCKET> m_userSockets;
};
} // namespace SERVER_NS