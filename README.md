# wsproxy

## Introduction
This is a WebSocket to TCP proxy for text-based protocols. WebSocket is a new technology for the communication between client-side JavaScript applications and backend servers. Since you cannot use JavaScript to connect to arbitrary TCP sockets directly, this proxy allows you to do so.

This only works for text-based protocols. Every WebSocket message is sent to the TCP server as a text line (a newline is appended automatically), and each text line from the server (delimited by a newline) is sent to the JavaScript application as one WebSocket message.

## Prerequisites
wsproxy uses boost::asio and [websocketpp](https://github.com/zaphoyd/websocketpp). The latter is automatically downloaded during the build process (using CMake's ExternalProject facilities).

## Building from source
The build system in use is CMake. After downloading the source code, navigate to the source directory and run

    mkdir build
    cd build
    cmake ..
    make

## Packaging
The project has been configured to use CPack to build a Debian package (Jessie and above). Sorry, no other packages at this time. To build the package, run `make package` inside the build directory.

## Running
Run `wsproxy --help`to get the help output:

    Allowed options:
      -h [ --help ]                         Show help message
      -t [ --target-host ] arg (=127.0.0.1) Target host
      -p [ --target-port ] arg (=4303)      Target port
      -w [ --websocket-port ] arg (=9002)   WebSocket port
      --pidfile arg                         PID file
      --daemon                              Daemonize

## Installing as a system service
There is an example init.d run script for Debian Jessie in `etc/init.d`. To install it, navigate to the source directory and run:

    sudo cp etc/init.d/wsproxy /etc/init.d
    update-rc.d wsproxy defaults

## License
2-clause "simplified" BSD. See LICENSE file for details.

## Known issues
* Most of wsproxy runs in an asynchronuous fashion and should be performing pretty well, with one exception: Opening a new connection blocks the entire application until the connection is established or fails.
* If connecting to the TCP backend fails, the WebSocket is not closed.
* This application has not been tested for security holes. Do not use on a public server! All of this comes without warranty. See LICENSE file for details.
