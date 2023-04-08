/**
		CROSSPLATFORM NET++.
  ------------------------------------------------------
	   Copyright (c) (*), QMV Corporation.
  ------------------------------------------------------
* @file:        server.hpp
* @brief:       Crossplatform Network(TCP) available for Windows/Linux (x64)
* @issue:	#1
		https://github.com/ynwqmv/netprotocol/issues/1
* @release:     0x79C2-venera
		https://github.com/ynwqmv/netprotocol/releases
* @modified:    -
* @author:      @ynwqmv
* @version:     0x79C2 && 0x79C2L
* @network_info:Crossplatform with GUI and WEB Explorers
**/




# pragma once
# ifndef NETWORK_SERVER_HPP

# define NETWORK_SERVER_HPP


#if defined(_WIN32) // Windows
# if _HAS_CXX20
# define __NETV 0x79C2        // *** v31170 *** 
# endif
#elif defined(__linux__) // Linux
# define __NETV 0x79C2L 	
#endif



#if defined(__NETV)
#include <fstream>

# include <boost/asio.hpp>
# include <spdlog/spdlog.h>


# include "../system/except.hpp"
# include "../crypto/sha256.h"

# include "../crypto/sha256.cpp"
# include <memory>
# include <chrono>
# include <sstream>
# include <unordered_map>


#ifdef _WIN32
# include <format>

# else
# include <fmt/format.h> 
# endif


#ifdef _WIN32
typedef unsigned short int uint;

#endif
using boost::asio::ip::tcp;
/*
	* @brief: inheritance-blocked.
		* using for hashing(SHA256) host and port
		* @more: https://github.com/ynwqmv/netprotocol/blob/master/README.md
*/
struct HHash final
{
	std::string host;	   /* host */
	std::string port;	   /* port */
	std::stringstream make;/* host + ':' + port */
	std::string make_hash; /* sha256(make) */

}host_t;


class Server
{

public:

	explicit Server(uint _port)
		: io_context(), port(_port),
		acceptor(io_context, tcp::endpoint(tcp::v4(), port))
	{
		spdlog::info("Server is running on port: {}", port);
		update();  // updating server
		listen_for_connections(); // listenning for new connections and accepting to the server
	}
	/*
		* @brief: runs the network. in-built nodejs require.
	*/
	void run()
	{
#if defined(_WIN32) 
		int err_code = system("node --version");
		if (err_code == 0)
		{
			spdlog::info("NodeJS is installed. [ok]");
			_is_installed_nodejs = true;
		}
		else
		{
			spdlog::critical("NodeJS is not installed. Please, install NodeJS client in your pc!. [warn: R100]");
			_is_installed_nodejs = false;
		}

#elif defined(__linux__)
		int err_code = system("node --version");
		if (err_code == 0)
		{
			spdlog::info("NodeJS is installed. [ok]");

			_is_installed_nodejs = true;
		}
		else {
			spdlog::critical("NodeJS is not installed.[warn: R100].  Do you want to install it automatically now? (y/n): ");
			char _tmp;
			std::cin >> _tmp;
			if (_tmp == 'y' or _tmp == 'Y')
			{
				system("sudo apt-get update");
				int if_success = system("sudo apt-get install nodejs");
				if (if_success == 0) { spdlog::info("NodeJS has been installed  successfully"); }
				else { spdlog::critical("abort"); }

			}

		}
#endif

		io_context.run();

	}

private:

	/*
		* @brief: accepting new connections
	*/
	void listen_for_connections()
	{

		auto socket = std::make_shared<tcp::socket>(io_context);
		acceptor.async_accept(*socket, [this, socket](const boost::system::error_code& error) {
			if (!error) {


				sockets.emplace_back(socket);
				host_data.host = socket->remote_endpoint().address().to_string();
				host_data.port = std::to_string(socket->remote_endpoint().port());

				host_data.make << host_data.host << ':' << host_data.port;
				host_data.make_hash = sha256(host_data.make.str());
#ifdef _WIN32

				std::string filename = "db.txt";
				SetFileAttributesA(filename.c_str(), FILE_ATTRIBUTE_NORMAL);
				std::ofstream file;
				file.open(filename, std::ios::app);
				if (file.is_open())
				{
					file << host_data.make_hash << "\r\n";
				}
				else
				{
					try
					{
#if defined(_WIN32)
						throw netv::stream_error(std::format("Couldn't add data in `{}`", filename));
#elif defined(__linux__)
						throw netv::stream_error(fmt::format("Couldn't add data in `{}`", filename));
#endif
					}
					catch (netv::stream_error& re)
					{
						spdlog::warn("fstream: {}", re.what());
					}
				}
				file.close();
				SetFileAttributesA(filename.c_str(), FILE_ATTRIBUTE_READONLY);

#elif defined(__linux__)

				std::string filename = "db.txt";
				system("chmod 644 db.txt"); // set read & write
				std::ofstream file;
				file.open(filename, std::ios::app);
				if (file.is_open())
				{
					file << host_data.make_hash << "\r\n";
				}
				else
				{
					try
					{
#if defined(_WIN32)
						throw netv::stream_error(std::format("Couldn't add data in `{}`", filename));
#elif defined(__linux__)					
						throw netv::stream_error(fmt::format("Couldn't add data in `{}`", filename));
#endif
					}
					catch (netv::stream_error& re)
					{
						spdlog::warn("fstream: {}", re.what());
					}
				}
				file.close();
				system("chmod 444 db.txt"); // set read-only 

#endif
				spdlog::info("> New connection established: {}", host_data.make_hash);
				for (const auto& sock : sockets)
				{
#if defined(_WIN32)
					boost::asio::write(*sock,
						boost::asio::buffer(std::format("> New user has joined: {}\n", host_data.make_hash)));
#elif defined(__linux__)
					boost::asio::write(*sock,
						boost::asio::buffer(fmt::format("> New user has joined: {}\n", host_data.make_hash)));
#endif
				}
				;
				data[host_data.make_hash] = socket;
				++online;

				listen_for_messages(socket, host_data); // pass the host_data struct to listen_for_messages
				listen_for_connections();
			}
			else
			{
				spdlog::error("> Error while accepting connection: {}", error.message());

			}
			});
	}


	inline void listen_for_messages(std::shared_ptr<tcp::socket> socket, HHash& host_data)
	{

		const auto message = std::make_shared<std::string>();
		boost::asio::async_read_until(*socket, boost::asio::dynamic_buffer(*message), '\n',
			[this, socket, message, &host_data](const boost::system::error_code& error, std::size_t length)
			{
				if (!error)
				{
					handleMessage(*message, socket);
					for (const auto& sock : sockets)
					{
						if (sock != socket)
						{
							if (message->substr(0, 5) == "/send" or message->substr(0, 6) == "/hello"
								or message->substr(0, 6) == "/socks" or message->substr(0, 7) == "/online"
								or message->substr(0, 5) == "/info")
							{
								/* => nothing: */
							}
							else
							{

#if defined(_WIN32)
								boost::asio::write(*sock, boost::asio::buffer(std::format("\r\n[{}]", std::to_string(socket->remote_endpoint().port()))));
#elif defined(__linux__)
								boost::asio::write(*sock, boost::asio::buffer(fmt::format("\r\n[{}]", std::to_string(socket->remote_endpoint().port()))));
#endif
								boost::asio::write(*sock, boost::asio::buffer(*message));
							}

						}
					}
					listen_for_messages(socket, host_data);
				}
				else if (error == boost::asio::error::eof)
				{

					for (auto it = data.begin(); it != data.end(); ++it)
					{
						if (it->second == socket)
						{
							/*
								Explanation : => we iterate in `data` - (unordered_map), when we meet socket
								which is equal to socket in parameter (that means, we got that socket,
								which disconnected), so we erasing it from `data` container and `sockets` container
								Fixed : => program crush while new connection was sending message to chat
							*/
							spdlog::critical("{} disconnected\r\n", it->first);
							for (const auto& sock : sockets) // sending to all users a message about client status
							{
								if (sock != socket)
								{
#if defined(_WIN32)
									boost::asio::write(*sock,
										boost::asio::buffer(std::format("> Client {} disconnected", it->first)));
#elif defined(__linux__)
									boost::asio::write(*sock,
										boost::asio::buffer(fmt::format("> Client {} disconnected", it->first)));
#endif
								}
							}
							--online;
							data.erase(it);
							sockets.erase(std::remove(sockets.begin(), sockets.end(), socket), sockets.end());
							break;
						}
					}
				}
				else
				{
					spdlog::error("Error while handle new connection from {}", /* If there are some error_code, we're telling to server */
						socket->remote_endpoint().port());
					for (auto it = data.begin(); it != data.end(); ++it)
					{
						if (it->second == socket)
						{
							spdlog::critical("{} disconnected\r\n", it->first);
							for (const auto& sock : sockets) // sending to all users a message about client status
							{
								if (sock != socket)
								{
#if defined(_WIN32)
									boost::asio::write(*sock,
										boost::asio::buffer(std::format("> Client {} disconnected", it->first)));
#elif defined(__linux__)
									boost::asio::write(*sock,
										boost::asio::buffer(fmt::format("> Client {} disconnected", it->first)));
#endif
								}
							}
							--online;

							data.erase(it);

							sockets.erase(std::remove(sockets.begin(), sockets.end(), socket), sockets.end());
							break;
						}
					}
				}
			});
	}


	inline void handleMessage(std::string& message, std::shared_ptr<tcp::socket> socket) const
	{
		if (message.substr(0, 6) == "/hello")
		{
			for (const auto& sock : sockets)
			{

				if (sock != socket)
				{
#if defined(_WIN32)
					auto message = std::make_shared<std::string>(std::format("\033[42mHELLO EVERYONE! FROM: [{}]\033[0m\n\r\n",
						std::to_string(sock->remote_endpoint().port())));
#elif defined(__linux__)
					auto message = std::make_shared<std::string>(fmt::format("\033[42mHELLO EVERYONE! FROM: [{}]\033[0m\n\r\n",
						std::to_string(sock->remote_endpoint().port())));
#endif
					boost::asio::async_write(*sock, boost::asio::buffer(*message),
						[socket, message](const boost::system::error_code& error, std::size_t bytes_transferred) {
							if (error) {
								std::cerr << "Error while sending message: " << error.message() << std::endl;
							}
						});
				}

			}
		}
		else if (message.substr(0, 7) == "/online")
		{
			for (const auto& sock : sockets)
			{
				if (sock == socket)
				{
#if defined(_WIN32)
					auto message = std::make_shared<std::string>(std::format("\033[42mOnline: {}\033[0m\n\r\n", std::to_string(online)));
#elif defined(__linux__)
					auto message = std::make_shared<std::string>(fmt::format("\033[42mOnline: {}\033[0m\n\r\n", std::to_string(online)));
#endif
					boost::asio::async_write(*sock, boost::asio::buffer(*message),
						[socket, message](const boost::system::error_code& error, std::size_t bytes_transferred) {
							if (error) {
								std::cerr << "Error while sending message: " << error.message() << std::endl;
							}
						});
				}
			}
		}
		else if (message.substr(0, 6) == "/socks")
		{
			print_data(socket);

		}
		else if (message.substr(0, 5) == "/info")
		{
			if (_is_installed_nodejs == true)
			{
				for (const auto& sock : sockets)
				{
					if (sock == socket)
					{
#if defined(_WIN32)
						auto message = std::make_shared<std::string>(std::format("\033[42mNodeJS: installed ({})\nVersion: {}\nPlatform: Windows ({})\nPlatform: Linux ({})\nOnline: {}\033[0m\n\r\n",
							_is_installed_nodejs, __NETV, _WIN32, false, online));
#elif defined(__linux__)
						auto message = std::make_shared<std::string>(fmt::format("\033[42mNodeJS: installed ({})\nVersion: {}\nPlatform: Windows ({})\nPlatform: Linux ({})\nOnline: {}\033[0m\n\r\n",
							_is_installed_nodejs, __NETV, false, __linux__, online));
#endif 
						boost::asio::async_write(*sock, boost::asio::buffer(*message),
							[socket, message](const boost::system::error_code& error, std::size_t bytes_transferred) {
								if (error) {
									std::cerr << "Error while sending message: " << error.message() << std::endl;
								}
							});
					}
				}
			}
			else
			{
				for (const auto& sock : sockets)
				{
					if (sock == socket)
					{
						spdlog::warn("NodeJS: not installed. ({})", _is_installed_nodejs);
					}
				}

			}
		}
		else if (message.substr(0, 5) == "/send")
		{
			std::istringstream iss(message);
			std::string command, recipient, text;
			iss >> command >> recipient;
			std::getline(iss, text);
			recipient.erase(0, recipient.find_first_not_of(" "));

			recipient.erase(recipient.find_last_not_of(" ") + 1);

			auto it = data.find(recipient);
			if (it != data.end())
			{
#if defined(_WIN32)
				auto message = std::make_shared<std::string>(std::format("\033[46mPrivate message from [{}] : {}\033[0m\n\r\n",
					std::to_string(socket->remote_endpoint().port()), text));
#elif defined(__linux__)
				auto message = std::make_shared<std::string>(fmt::format("\033[46mPrivate message from [{}] : {}\033[0m\n\r\n",
					std::to_string(socket->remote_endpoint().port()), text));

#endif
				boost::asio::async_write(*it->second, boost::asio::buffer(*message),
					[socket, message](const boost::system::error_code& error, std::size_t bytes_transferred) {
						if (error) {
							std::cerr << "Error while sending message: " << error.message() << std::endl;
						}
					});
			}
			else
			{
#if defined(_WIN32)

				auto message = std::make_shared < std::string >
					(std::format("\033[41mError: user `{}` not found. Try `/socks` to discover all connected users\033[0m\n\r\n", recipient));
#elif defined(__linux__)
				auto message = std::make_shared < std::string >
					(fmt::format("\033[41mError: user `{}` not found. Try `/socks` to discover all connected users\033[0m\n\r\n", recipient));
#endif
				boost::asio::async_write(*socket,
					boost::asio::buffer(*message), [this, message, socket](const boost::system::error_code& error, std::size_t bytes_transferred) {
						if (error)
						{
							std::cerr << "Error while sending message: " << error.message() << std::endl;
							for (const auto& sock : sockets)
							{
								if (sock == socket)
								{
#if defined(_WIN32)
									boost::asio::write(*sock,
										boost::asio::buffer(std::format("Error while sending message: {}",
											error.message())));

#elif defined(__linux__)
									boost::asio::write(*sock,
										boost::asio::buffer(fmt::format("Error while sending message: {}",
											error.message())));
#endif
								}
							}
						}
						else
						{
#if defined(_WIN32)
							auto smessage = std::make_shared<std::string>(std::format("\r\n[{} bytes]", bytes_transferred));
#elif defined(__linux__)
							auto smessage = std::make_shared<std::string>(fmt::format("\r\n[{} bytes]", bytes_transferred));
#endif
							for (const auto& sock : sockets)
							{
								if (sock == socket)
								{
									boost::asio::write(*sock,
										boost::asio::buffer(*smessage));
								}
							}
						}
					});

			}
		}
	}

	/*
	* @brief: prints connected sockets' hash to server
	*/
	inline void print_data_server() const
	{
		/*
			* for_each
		*/
		for (const auto& _data : data)
		{
			spdlog::info("Users connected: {}", _data.first);
		}
	}
	/*
		*  @brief: each connected users see the message that someone has been connected.
	*/
	inline void print_data(const std::shared_ptr<tcp::socket>& socket) const
	{
		for (const auto& sock : sockets)   /// Iterates through all sockets
		{
			if (sock == socket)            /// If iterated socket => socket from parameter
			{
				for (const auto& _data : data)  /// Than we iterate through `data` container and write data for the socket which requested the command
				{
					boost::asio::write(*sock, boost::asio::buffer("\033[42mConnected: " + _data.first + "\033[0m\n\r\n"));
				}
			}
		}
	}
	/* @brief: each 30 seconds we calling update() with its data */
	inline void update() {
		auto timer = std::make_shared<boost::asio::steady_timer>(io_context);
		timer->expires_after(std::chrono::seconds(30));
		timer->async_wait([this, timer](const boost::system::error_code& error) {
			if (!error) {
				// Perform the update operation here
				spdlog::info("Online: {}", std::to_string(online));

				for (const auto& sock : sockets)
				{
#if defined(_WIN32)
					boost::asio::write(*sock,
						boost::asio::buffer(std::format("Online: {}", std::to_string(online))));

#elif defined(__linux__)
					boost::asio::write(*sock,

						boost::asio::buffer(fmt::format("Online: {}", std::to_string(online))));

#endif
				}
				// Reset the timer to expire after another 10 seconds
				print_data_server();
				update();
			}
			else {
				spdlog::error("Error in timer callback: {}", error.message());

			}
			});
	}

private:
	/*~~~~~~~~~~~~~BOOST~~~~~~~~~~~~~~*/
	boost::asio::io_context io_context;
	uint port;
	HHash host_data;
	tcp::acceptor acceptor;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~CONTAINERS~~~~~~~~~~~*/
	std::vector<std::shared_ptr<tcp::socket>> sockets;
	std::unordered_map<std::string, std::shared_ptr<tcp::socket>> data;
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	/*~~~~~~~~~~~~~STATS~~~~~~~~~~~~~~*/
	static unsigned short online;
	bool _is_installed_nodejs = false; // boolean var for checking nodejs installation 
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
};

unsigned short Server::online = 0;
# endif  // __NETV
# endif // NETWORK_SERVER_HPP