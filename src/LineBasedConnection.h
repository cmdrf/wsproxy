/*
Copyright (c) 2015, Fabian Herb
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

#ifndef LINEBASEDCONNECTION_H
#define LINEBASEDCONNECTION_H

#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>

/// Connection to a TCP server speaking a text-based protocol
class LineBasedConnection : public std::enable_shared_from_this<LineBasedConnection>, boost::noncopyable
{
public:
	typedef LineBasedConnection Self;

	typedef std::function<void(const char*)> ResponseHandler;
	typedef std::function<void(boost::system::error_code)> ErrorHandler;
	typedef boost::asio::ip::tcp::resolver::iterator Destination;
	typedef boost::asio::io_service IoService;

	/// Constructor
	/** Warning: This function will block until the connection is established
		or an error occurs.
		@param ioService asio::io_service that handles this connection.
		@param destination Destination (host and port) to connect to.
		@param responseHandler Function to be called when data arrives from the server.
		@param errorHandler Function to be called on errors. */
	LineBasedConnection(IoService& ioService, Destination destination, ResponseHandler responseHandler, ErrorHandler errorHandler);

	~LineBasedConnection();
	void Start();
	void Stop() {mStopped = true; mSocket.close();}

	/// Send a line of text
	/** A newline is automatically appended. */
	void Send(const std::string& command);

	/// Set function to be called on arrival of a new text line
	void SetResponseHandler(ResponseHandler& handler) {mResponseHandler = handler;}

private:
	void ReadResponse();
	void OnResponse(boost::system::error_code ec, std::size_t length);

	boost::asio::ip::tcp::socket mSocket;
	boost::asio::streambuf mResponseBuffer;
	std::istream mResponseStream;
	ResponseHandler mResponseHandler;
	ErrorHandler mErrorHandler;
	bool mStopped;
};

#endif // LINEBASEDCONNECTION_H

