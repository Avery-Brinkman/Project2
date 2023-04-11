#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <map>
#include <vector>

#include "group.h"

constexpr auto DEFAULT_BUFLEN = 512;

namespace SERVER_NS {
class Server {
public:
  explicit Server(const char* port = "5000");

  void shutdown();

private:
  void clientHandler(const std::string_view userName, const SOCKET& userSocket) const;

  std::vector<GROUP_NS::Group> m_groups;

  SOCKET m_listenSocket = INVALID_SOCKET;

  // needed?
  std::map<std::string, SOCKET> m_userSockets;
};
} // namespace SERVER_NS