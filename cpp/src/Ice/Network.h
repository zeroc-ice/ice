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

#ifndef WIN32
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

bool interrupted();
bool acceptInterrupted();
bool noBuffers();
bool wouldBlock();
bool connectFailed();
bool connectInProgress();
bool connectionLost();
bool notConnected();

SOCKET createSocket(bool);
void closeSocket(SOCKET);

void setBlock(SOCKET, bool);
void setTcpNoDelay(SOCKET);
void setKeepAlive(SOCKET);
void setSendBufferSize(SOCKET, int);

void doBind(SOCKET, struct sockaddr_in&);
void doListen(SOCKET, int);
void doConnect(SOCKET, struct sockaddr_in&, int);
SOCKET doAccept(SOCKET, int);

void getAddress(const std::string&, int, struct sockaddr_in&);

void createPipe(SOCKET fds[2]);

std::string errorToString(int);
std::string errorToStringDNS(int);
std::string lastErrorToString();
std::string lastErrorToStringDNS();

std::string fdToString(SOCKET);
std::string addrToString(const struct sockaddr_in&);

}

#endif
