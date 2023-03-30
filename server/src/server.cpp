#include <algorithm>
#include <iostream>
#include "server.h"

using boost::asio::ip::tcp;
using namespace SERVER_NS;

Server::Server(boost::asio::io_context& io_context) :
  m_io_context(io_context), m_acceptor(tcp::acceptor(m_io_context, tcp::endpoint(tcp::v4(), PORT))), m_groups(std::vector<GROUP_NS::Group>(6)) {
  std::cout << "Server running on port " << PORT << std::endl;

  while (true) {
    std::cout << "Waiting for connections..." << std::endl;
    tcp::socket socket(m_io_context);
    m_acceptor.accept(socket);

    std::cout << "Connection established! Reading data..." << std::endl;

    boost::asio::streambuf buf;
    boost::asio::read_until(socket, buf, "\n");

    std::string message = boost::asio::buffer_cast<const char*>(buf.data());
    std::cout << "Recieved: " << message;
    std::transform(message.begin(), message.end(), message.begin(), ::toupper);
    std::cout << "Returning: " << message;

    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(message), ignored_error);
  }
}
