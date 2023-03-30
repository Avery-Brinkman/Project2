#pragma once

#include <boost/asio.hpp>
#include <map>
#include <string>

namespace GROUP_NS {
  struct Message {
    int id;
    std::string userName;
    std::string subject;
    std::string message;
  };

  class Group {
  public:
    Group() = default;

    void addUser(const std::string_view userName);

    void removeUser(const std::string_view userName);

    int postMessage(const std::string_view name, const std::string_view subject, const std::string_view content);

    Message getMessage(const int message);

  private:
    std::map<std::string, boost::asio::ip::tcp::socket> m_users;
  };
}