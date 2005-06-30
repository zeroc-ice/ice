// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_NETWORK_H
#define ICE_NETWORK_H

#ifdef __hpux
#   define _XOPEN_SOURCE_EXTENDED
#endif

#include <Ice/Config.h>

#ifdef _WIN32
#   include <winsock.h>
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

ICE_PROTOCOL_API bool interrupted();
ICE_PROTOCOL_API bool acceptInterrupted();
ICE_PROTOCOL_API bool noBuffers();
ICE_PROTOCOL_API bool wouldBlock();
ICE_PROTOCOL_API bool connectFailed();
ICE_PROTOCOL_API bool connectionRefused();
ICE_PROTOCOL_API bool connectInProgress();
ICE_PROTOCOL_API bool connectionLost();
ICE_PROTOCOL_API bool notConnected();
ICE_PROTOCOL_API bool recvTruncated();

ICE_PROTOCOL_API SOCKET createSocket(bool);
ICE_PROTOCOL_API void closeSocket(SOCKET);
ICE_PROTOCOL_API void shutdownSocketWrite(SOCKET);
ICE_PROTOCOL_API void shutdownSocketReadWrite(SOCKET);

ICE_PROTOCOL_API void setBlock(SOCKET, bool);
ICE_PROTOCOL_API void setTcpNoDelay(SOCKET);
ICE_PROTOCOL_API void setKeepAlive(SOCKET);
ICE_PROTOCOL_API void setSendBufferSize(SOCKET, int);
ICE_PROTOCOL_API int getSendBufferSize(SOCKET);
ICE_PROTOCOL_API void setRecvBufferSize(SOCKET, int);
ICE_PROTOCOL_API int getRecvBufferSize(SOCKET);

ICE_PROTOCOL_API void doBind(SOCKET, struct sockaddr_in&);
ICE_PROTOCOL_API void doListen(SOCKET, int);
ICE_PROTOCOL_API void doConnect(SOCKET, struct sockaddr_in&, int);
ICE_PROTOCOL_API SOCKET doAccept(SOCKET, int);

ICE_PROTOCOL_API void getAddress(const std::string&, int, struct sockaddr_in&);
ICE_PROTOCOL_API std::string getLocalHost(bool);
ICE_PROTOCOL_API bool compareAddress(const struct sockaddr_in&, const struct sockaddr_in&);

ICE_PROTOCOL_API void createPipe(SOCKET fds[2]);

ICE_PROTOCOL_API std::string errorToString(int);
ICE_PROTOCOL_API std::string errorToStringDNS(int);
ICE_PROTOCOL_API std::string lastErrorToString();

ICE_PROTOCOL_API std::string fdToString(SOCKET);
ICE_PROTOCOL_API std::string addrToString(const struct sockaddr_in&);

#ifdef _WIN32
ICE_PROTOCOL_API std::vector<struct sockaddr_in> getLocalAddresses();
ICE_PROTOCOL_API bool isLocalAddress(const struct sockaddr_in&);
ICE_PROTOCOL_API bool isPeerLocal(SOCKET);
#endif

}

#endif
