#include <iostream>
#include "server.h"

using boost::asio::ip::tcp;
using namespace SERVER_NS;

Server::Server(boost::asio::io_context& io_context) :
  m_io_context(io_context), m_acceptor(tcp::acceptor(m_io_context, tcp::endpoint(tcp::v4(), PORT))) {
  std::cout << "Server running on port " << PORT << std::endl;

  while (true) {
    std::cout << "Waiting for connections..." << std::endl;
    tcp::socket socket(m_io_context);
    m_acceptor.accept(socket);

    std::cout << "Connection established!" << std::endl;

    std::string message = "Success!";

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
  }
}
