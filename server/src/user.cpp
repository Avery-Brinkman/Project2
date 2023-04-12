#include "user.h"

using namespace USER_NS;

User::User(const std::string_view userName, const SOCKET& userSocket, const int groupCount) {
  name = std::string(userName.data(), userName.size());
  socket = userSocket;
  m_groups = std::vector<GROUP_NS::Group>(groupCount);
}