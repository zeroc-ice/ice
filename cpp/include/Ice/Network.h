// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_NETWORK_H
#define ICE_NETWORK_H

#include <Ice/Config.h>

#ifdef WIN32
#   include <winsock.h>
#else
#   include <unistd.h>
#   include <fcntl.h>
#   include <sys/socket.h>
#   include <sys/select.h>
#   include <netinet/in.h>
#   include <netinet/tcp.h>
#   include <arpa/inet.h>
#   include <netdb.h>
#endif

#ifdef WIN32
typedef int socklen_t;
#endif

#ifndef SOCKET_ERROR
#   define SOCKET_ERROR -1
#endif

#ifndef INVALID_SOCKET
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

namespace _Ice
{

bool interrupted();
bool acceptInterrupted();
bool connectFailed();
bool connectionLost();
void setBlock(int, bool);
void closeSocket(int);
void setTcpNoDelay(int);
void setKeepAlive(int);
void getHostByName(const char*, int, struct sockaddr_in&);
void doBind(int, struct sockaddr_in&);
void doListen(int, int);
void doConnect(int, struct sockaddr_in&);
int doAccept(int);
void createPipe(int fds[2]);
const char* errorToString(int);
const char* errorToStringDNS(int);
const char* lastErrorToString();
const char* lastErrorToStringDNS();
std::string fdToString(int);
std::string addrToString(struct sockaddr_in&);

}

#endif
