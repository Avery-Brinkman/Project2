#pragma once

#include "group.h"

namespace SERVER_NS {
  class Server {
  public:
    Server() = default;
  private:
    GROUP_NS::Group m_publicGroup;
  };
}