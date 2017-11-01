cmake_minimum_required(VERSION 3.0)
project(wsproxy)

set(WS_PROXY_SOURCES
	src/WsProxyMain.cpp
	src/LineBasedConnection.cpp
	src/PidFile.cpp
)

set(WS_PROXY_HEADERS
	src/LineBasedConnection.h
	src/PidFile.h
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
if(CMAKE_TOOLCHAIN_FILE)
	get_filename_component(CMAKE_TOOLCHAIN_FILE_ABS "${CMAKE_TOOLCHAIN_FILE}" ABSOLUTE)
endif()

find_package(Boost REQUIRED COMPONENTS system program_options)

include(ExternalProject)
set(DEPENDENCIES_PREFIX ${CMAKE_BINARY_DIR}/dependencies)
ExternalProject_Add(websocketpp
	PREFIX ${CMAKE_BINARY_DIR}/websocketpp
	URL https://github.com/zaphoyd/websocketpp/archive/0.7.0.tar.gz
	URL_MD5 5027c20cde76fdaef83a74acfcf98e23
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCIES_PREFIX}
		-DCMAKE_TOOLCHAIN_FILE:PATH=${CMAKE_TOOLCHAIN_FILE_ABS}
)
set_target_properties(websocketpp PROPERTIES EXCLUDE_FROM_ALL TRUE)

include_directories(SYSTEM
	${DEPENDENCIES_PREFIX}/include
	${Boost_INCLUDE_DIRS}
)
include_directories(src)

add_executable(wsproxy ${WS_PROXY_SOURCES} ${WS_PROXY_HEADERS})
add_dependencies(wsproxy websocketpp)
target_link_libraries(wsproxy ${Boost_LIBRARIES})
if(UNIX AND NOT APPLE)
	target_link_libraries(wsproxy -pthread)
endif()

# Install:
install(TARGETS wsproxy DESTINATION bin)

# Build packet:
execute_process(COMMAND dpkg --print-architecture OUTPUT_VARIABLE _DPKG_ARCH ERROR_QUIET)
if(_DPKG_ARCH)
	string(STRIP ${_DPKG_ARCH} DPKG_ARCH)
	set(CPACK_SYSTEM_NAME ${DPKG_ARCH})
endif()
set(BOOST_PKG_VER "${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}.${Boost_SUBMINOR_VERSION}")

set(CPACK_PACKAGE_NAME "wsproxy")
set(CPACK_PACKAGE_CONTACT "Fabian Herb <fabian@herb-clan.de>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "WebSocket to TCP proxy")
set(CPACK_PACKAGE_VERSION "0.1")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libboost-system${BOOST_PKG_VER}, libboost-program-options${BOOST_PKG_VER}")
#set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_CURRENT_SOURCE_DIR}/conf/postinst)
set(CPACK_GENERATOR "DEB")
set(CPACK_IGNORE_FILES ".*")
include(CPack)
