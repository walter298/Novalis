#pragma once

#include <algorithm>
#include <array>
#include <functional>
#include <ranges>
#include <string>

#include <boost/asio.hpp>

namespace nv {
	struct Message {
		std::string title;
		std::string body;

		Message() = default;

		template<std::ranges::viewable_range Buff>
		explicit Message(const Buff& buff) {
			//parse title
			//auto newLinePos = std::ranges::find(buff, '\n');
			//assert(newLinePos != buff.end());
			//std::copy(buff.begin(), newLinePos, title.begin());
			
			//parse body
			//auto bodyBeginPos = std::next(newLinePos);
			/*if (bodyBeginPos != buff.cend()) {
				std::ranges::copy(std::next(newLinePos), buff.end(), body.begin());
			}*/
		}
	};

	namespace asio = boost::asio;
	using asio::ip::tcp;

	class Connection {
	private:
		static constexpr size_t MAX_MESSAGE_LEN = 8192;
		tcp::socket m_socket;
		asio::io_context& m_context;
	public:
		Connection(asio::io_context& context, const tcp::endpoint& endpoint);
		void send(const Message& msg);
		Message recv();
	};
}