// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_ACCEPTOR_H
#define ICE_SSL_ACCEPTOR_H

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

class SslEndpoint;

class SslAcceptor : public Acceptor
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

    SslAcceptor(const InstancePtr&, int);
    virtual ~SslAcceptor();
    friend class SslEndpoint;

    InstancePtr _instance;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    int _fd;
    int _backlog;
    struct sockaddr_in _addr;
};

}

#endif
