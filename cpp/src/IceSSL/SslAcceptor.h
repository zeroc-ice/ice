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

#ifndef _WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceSSL
{

class SslEndpoint;

class SslAcceptor : public IceInternal::Acceptor
{
public:

    virtual SOCKET fd();
    virtual void close();
    virtual void listen();
    virtual IceInternal::TransceiverPtr accept(int);
    virtual std::string toString() const;

    bool equivalent(const std::string&, int) const;
    int effectivePort();

private:

    SslAcceptor(const IceInternal::InstancePtr&, const std::string&, int);
    virtual ~SslAcceptor();
    friend class SslEndpoint;

    IceInternal::InstancePtr _instance;
    IceInternal::TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_in _addr;
};

}

#endif
