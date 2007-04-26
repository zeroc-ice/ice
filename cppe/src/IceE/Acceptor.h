// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ACCEPTOR_H
#define ICEE_ACCEPTOR_H

#include <IceE/TransceiverF.h>
#include <IceE/InstanceF.h>
#include <IceE/TraceLevelsF.h>
#include <IceE/LoggerF.h>
#include <IceE/AcceptorF.h>

#include <IceE/Shared.h>

#ifdef _WIN32
#   include <winsock2.h>
typedef int ssize_t;
#else
#   define SOCKET int
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class Endpoint;

class Acceptor : public ::IceUtil::Shared
{
public:

    SOCKET fd();
    void close();
    void listen();
    TransceiverPtr accept();
    void connectToSelf();
    std::string toString() const;

    int effectivePort();

private:

    Acceptor(const InstancePtr&, const std::string&, int);
    virtual ~Acceptor();
    friend class TcpEndpoint;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_in _addr;
};

}

#endif
