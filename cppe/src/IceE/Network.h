// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_NETWORK_H
#define ICE_NETWORK_H

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

bool interrupted();
bool acceptInterrupted();
bool noBuffers();
bool wouldBlock();
bool connectFailed();
bool connectionRefused();
bool connectInProgress();
bool connectionLost();
bool notConnected();

SOCKET createSocket();
void closeSocket(SOCKET);
void shutdownSocketWrite(SOCKET);
void shutdownSocketReadWrite(SOCKET);

void setBlock(SOCKET, bool);
void setTcpNoDelay(SOCKET);
void setKeepAlive(SOCKET);
void setSendBufferSize(SOCKET, int);

void doBind(SOCKET, struct sockaddr_in&);
void doListen(SOCKET, int);
void doConnect(SOCKET, struct sockaddr_in&, int);
SOCKET doAccept(SOCKET, int);

void getAddress(const std::string&, int, struct sockaddr_in&);
std::string getLocalHost(bool);
bool compareAddress(const struct sockaddr_in&, const struct sockaddr_in&);

std::string errorToString(int);
std::string errorToStringDNS(int);
std::string lastErrorToString();

std::string fdToString(SOCKET);
std::string addrToString(const struct sockaddr_in&);

#ifdef _WIN32
std::vector<struct sockaddr_in> getLocalAddresses();
bool isLocalAddress(const struct sockaddr_in&);
bool isPeerLocal(SOCKET);
#endif

int getSocketErrno();

}

#endif
