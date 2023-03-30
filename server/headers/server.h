#pragma once

#include <map>
#include <vector>
#include <boost/asio.hpp>
#include "group.h"

namespace SERVER_NS {
  static const unsigned int PORT = 5000;

  class Server {
  public:
    Server(boost::asio::io_context& io_context);

  private:
    std::vector<GROUP_NS::Group> m_groups;

    std::map<std::string, boost::asio::ip::tcp::socket> m_users;

    boost::asio::io_context& m_io_context;

    boost::asio::ip::tcp::acceptor m_acceptor;
  };
}