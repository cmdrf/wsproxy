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

#include <iostream>
#include "LineBasedConnection.h"

using boost::asio::ip::tcp;

LineBasedConnection::LineBasedConnection(IoService &ioService, Destination destination, ResponseHandler responseHandler, ErrorHandler errorHandler) :
	mSocket(ioService),
	mResponseStream(&mResponseBuffer),
	mResponseHandler(responseHandler),
	mErrorHandler(errorHandler),
	mStopped(false)
{
	mSocket.connect(*destination);
}

LineBasedConnection::~LineBasedConnection()
{

}

void LineBasedConnection::Start()
{
	ReadResponse();
}

void LineBasedConnection::Send(const std::string& msg)
{
	std::string toSend = msg + "\n";
	boost::asio::write(mSocket, boost::asio::buffer(toSend.data(), toSend.length()));
}

void LineBasedConnection::ReadResponse()
{
	using namespace std::placeholders;
	auto handler = std::bind(&LineBasedConnection::OnResponse, shared_from_this(), _1, _2);
	boost::asio::async_read_until(mSocket, mResponseBuffer, '\n', handler);
}

void LineBasedConnection::OnResponse(boost::system::error_code ec, std::size_t /*length*/)
{
	if(mStopped)
		return;

	if(!ec)
	{
		char buffer[1000];
		mResponseStream.getline(buffer, 1000);
		if(mResponseHandler)
			mResponseHandler(buffer);
		if(!mStopped)
			ReadResponse();
	}
	else
	{
		mStopped = true;
		if(mErrorHandler)
			mErrorHandler(ec);
	}
}
