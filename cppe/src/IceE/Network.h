// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_NETWORK_H
#define ICEE_NETWORK_H

#ifdef __hpux
#   define _XOPEN_SOURCE_EXTENDED
#endif

#include <IceE/Config.h>

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/socket.h>

#   if defined(__hpux)
#      include <sys/time.h>
#   else   
#      include <sys/select.h>
#   endif

#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#ifdef _WIN32
typedef int socklen_t;
#endif

#ifndef _WIN32
#   define SOCKET int
#   define SOCKET_ERROR -1
#   define INVALID_SOCKET -1
#endif

#ifndef SHUT_RD
#   define SHUT_RD 0
#endif

#ifndef SHUT_WR
#   define SHUT_WR 1
#endif

#ifndef SHUT_RDWR
#   define SHUT_RDWR 2
#endif

#ifndef NETDB_INTERNAL
#   define NETDB_INTERNAL -1
#endif

#ifndef NETDB_SUCCESS
#   define NETDB_SUCCESS 0
#endif

namespace IceInternal
{

ICEE_API bool interrupted();
ICEE_API bool acceptInterrupted();
ICEE_API bool noBuffers();
ICEE_API bool wouldBlock();
ICEE_API bool connectFailed();
ICEE_API bool connectionRefused();
ICEE_API bool connectInProgress();
ICEE_API bool connectionLost();
ICEE_API bool notConnected();

ICEE_API SOCKET createSocket();
ICEE_API void closeSocket(SOCKET);
ICEE_API void shutdownSocketWrite(SOCKET);
ICEE_API void shutdownSocketReadWrite(SOCKET);

ICEE_API void setBlock(SOCKET, bool);
ICEE_API void setTcpNoDelay(SOCKET);
ICEE_API void setKeepAlive(SOCKET);
ICEE_API void setSendBufferSize(SOCKET, int);

ICEE_API void doBind(SOCKET, struct sockaddr_in&);
ICEE_API void doListen(SOCKET, int);
ICEE_API void doConnect(SOCKET, struct sockaddr_in&, int);
ICEE_API SOCKET doAccept(SOCKET, int);

ICEE_API void getAddress(const std::string&, int, struct sockaddr_in&);
ICEE_API std::string getLocalHost(bool);
ICEE_API bool compareAddress(const struct sockaddr_in&, const struct sockaddr_in&);

ICEE_API std::string errorToString(int);
ICEE_API std::string errorToStringDNS(int);
ICEE_API std::string lastErrorToString();

ICEE_API std::string fdToString(SOCKET);
ICEE_API std::string addrToString(const struct sockaddr_in&);

#ifdef _WIN32
ICEE_API std::vector<struct sockaddr_in> getLocalAddresses();
ICEE_API bool isLocalAddress(const struct sockaddr_in&);
ICEE_API bool isPeerLocal(SOCKET);
#endif

ICEE_API int getSocketErrno();

}

#endif
