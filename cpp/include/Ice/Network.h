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

namespace _Ice
{

bool interrupted();
bool acceptInterrupted();
bool connectFailed();
bool connectionLost();
void closeSocket(int);
void setTcpNoDelay(int);
void setKeepAlive(int);
void getHostByName(const char*, int, struct sockaddr_in&);

}

#endif
