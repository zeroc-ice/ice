// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_ACCEPTOR_H
#define ICE_TCP_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Acceptor.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class TcpEndpoint;

class TcpAcceptor : public Acceptor
{
public:

    virtual int fd();
    virtual void close();
    virtual void shutdown();
    virtual void listen();
    virtual TransceiverPtr accept(int);
    virtual std::string toString() const;

    virtual bool equivalent(const std::string&, int) const;

private:

    TcpAcceptor(const InstancePtr&, int);
    virtual ~TcpAcceptor();
    friend class TcpEndpoint;

    InstancePtr _instance;
    int _fd;
    int _backlog;
    struct sockaddr_in _addr;
#ifndef ICE_NO_TRACE
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
#endif
};

}

#endif
