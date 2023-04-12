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
    User(const std::string_view userName, const SOCKET& userSocket);

    void quit();

    std::vector<int> joinedGroups() const;

    void joinGroup(int groupId, std::shared_ptr<GROUP_NS::Group> group);

    void leaveGroup(int groupId);

    void notifyJoin(std::string_view userName, int groupId) const;

    void notifyLeave(std::string_view userName, int groupId) const;

    void showGroupMembers(int groupId) const;

    bool selfQuit() const { return m_quit; }

    std::string name;

    SOCKET socket;

  private:
    std::map<int, std::shared_ptr<GROUP_NS::Group>> m_groups;

    bool m_quit = false;
  };
};