// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_TCP_ACCEPTOR_H
#define ICE_TCP_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Acceptor.h>

#ifndef _WIN32
#   include <sys/socket.h> // For struct sockaddr_storage
#endif

namespace IceInternal
{

class TcpEndpoint;

class TcpAcceptor : public Acceptor
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void listen();
    virtual TransceiverPtr accept();
    virtual std::string toString() const;

    int effectivePort() const;

private:

    TcpAcceptor(const InstancePtr&, const std::string&, int);
    virtual ~TcpAcceptor();
    friend class TcpEndpointI;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_storage _addr;
};

}

#endif
