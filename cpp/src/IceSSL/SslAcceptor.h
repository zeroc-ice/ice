// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
