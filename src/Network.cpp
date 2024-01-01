#include "Network.h"

nv::Connection::Connection(asio::io_context& context, const tcp::endpoint& endpoints) 
	: m_context{ context }, m_socket{context }
{
	tcp::resolver resolver{ context };
	asio::connect(m_socket, resolver.resolve(endpoints));
}

void nv::Connection::send(const Message& msg) {
	std::string totalMsgStr = msg.title + '\n' + msg.body;
	asio::write(m_socket, asio::buffer(totalMsgStr));
}

nv::Message nv::Connection::recv() {
	std::string buff;
	asio::read(m_socket, asio::dynamic_buffer(buff), asio::transfer_all());
	return Message{ buff };
}