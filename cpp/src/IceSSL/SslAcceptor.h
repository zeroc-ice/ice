// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_ACCEPTOR_H
#define ICE_SSL_ACCEPTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/LoggerF.h>
#include <Ice/Acceptor.h>
#include <IceSSL/OpenSSLPluginIF.h>
#include <IceSSL/TraceLevelsF.h>

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

    SslAcceptor(const OpenSSLPluginIPtr&, const std::string&, int);
    virtual ~SslAcceptor();
    friend class SslEndpoint;

    OpenSSLPluginIPtr _plugin;
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    SOCKET _fd;
    int _backlog;
    struct sockaddr_in _addr;
};

}

#endif
