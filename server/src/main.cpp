#include <boost/asio.hpp>

#include "server.h"

int main() {
  boost::asio::io_context ctx;
  SERVER_NS::Server test(ctx);
  ctx.run();

  return 1;
}
