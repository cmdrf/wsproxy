/*
Copyright (c) 2015-2017, Fabian Herb
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <iostream>
#include <memory>
#include "LineBasedConnection.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <functional>
#include <boost/program_options.hpp>
#include "PidFile.h"

using websocketpp::connection_hdl;
using boost::asio::ip::tcp;

typedef websocketpp::server<websocketpp::config::asio> Server;

static std::map<connection_hdl, std::shared_ptr<LineBasedConnection>, std::owner_less<connection_hdl>> connections;
static boost::asio::io_service ioService;
static tcp::resolver::iterator resolverIterator;
static Server server;

/// Response handler for the line based connection
/** To be passed to the constructor of LineBasedConnection. */
void OnLineBasedResponse(connection_hdl hdl, const char* response)
{
	try
	{
		server.send(hdl, response, websocketpp::frame::opcode::text);
	}
	catch(...)
	{
		std::cerr << "Send failed" << std::endl;
	}
}

/// Error handler for the line based connection
/** To be passed to the constructor of LineBasedConnection. */
void OnLineBasedError(connection_hdl hdl, boost::system::error_code /*error*/)
{
	if(!hdl.expired())
		server.close(hdl, websocketpp::close::status::going_away, "Error on connection");
}

/// Open handler for the WebSocket
void OnOpen(connection_hdl hdl)
{
	auto responseHandler = std::bind(OnLineBasedResponse, hdl, std::placeholders::_1);
	auto errorHandler = std::bind(OnLineBasedError, hdl, std::placeholders::_1);

	try
	{
		std::shared_ptr<LineBasedConnection> c(new LineBasedConnection(ioService, resolverIterator, responseHandler, errorHandler));
		connections.insert(std::make_pair(hdl, c));
		c->Start();
	}
	catch(std::exception& e)
	{
		std::cerr << "Line-based connection failed: " << e.what() << std::endl;
		// TODO: Close WebSocket!
	}
}

/// Close handler for the WebSocket
void OnClose(connection_hdl hdl)
{
	auto conn = connections.find(hdl);
	if(conn != connections.end())
	{
		conn->second->Stop();
		connections.erase(conn);
	}
}

/// Message handler for the WebSocket
void OnMessage(websocketpp::connection_hdl hdl, Server::message_ptr msg)
{
	auto conn = connections.find(hdl);
	if(conn != connections.end())
		conn->second->Send(msg->get_payload());
}

int main(int argc, char** argv)
{
	namespace po = boost::program_options;

	try
	{
		// Variables to be filled by boost::program_options:
		std::string targetHost, targetPort;
		int webSocketPort = 9002;
		std::string pidFileName;

		po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "Show help message")
			("target-host,t", po::value<std::string>(&targetHost)->default_value("127.0.0.1"), "Target host")
			("target-port,p", po::value<std::string>(&targetPort)->default_value("4303"), "Target port")
			("websocket-port,w", po::value<int>(&webSocketPort)->default_value(9002), "WebSocket port")
			("pidfile", po::value<std::string>(&pidFileName), "PID file")
			("daemon", "Daemonize")
		;

		po::variables_map vm;
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);

		if(vm.count("help"))
		{
			std::cerr << desc << "\n";
			return 1;
		}

		// Daemonize if requested:
		if(vm.count("daemon"))
		{
			if(daemon(0, 0) != 0)
				throw std::runtime_error(std::string("daemon: ") + strerror(errno));
		}

		PidFile pidFile(pidFileName);

		tcp::resolver resolver(ioService);
		tcp::resolver::query query(tcp::v4(), targetHost, targetPort);
		resolverIterator = resolver.resolve(query);

		// Set event handlers:
		server.set_open_handler(&OnOpen);
		server.set_close_handler(&OnClose);
		server.set_message_handler(&OnMessage);

		// Start:
		server.init_asio(&ioService);
		server.listen(webSocketPort);
		server.start_accept();

		ioService.run();
	}
	catch(const websocketpp::exception& e)
	{
		std::cerr << "websocketpp::exception: " << e.code().category().message(e.code().value()) << std::endl;
		return 1;
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
