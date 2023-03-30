#pragma once

#include <boost/asio.hpp>
#include "group.h"

using boost::asio::ip::tcp;

namespace SERVER_NS {
  class Server {
  public:
    Server() = default;
  private:

    GROUP_NS::Group m_publicGroup;
  };
}