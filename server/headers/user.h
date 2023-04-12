#pragma once

#define WIN32_LEAN_AND_MEAN

#include <WS2tcpip.h>
#include <string>
#include <vector>

#include "group.h"

namespace USER_NS {
  class User {
  public:
    User(const std::string_view userName, const SOCKET& userSocket, const int groupCount);

    std::string name;

    SOCKET socket;

  private:
    std::vector<GROUP_NS::Group> m_groups;
  };
};