// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_SSL_CONNECTOR_H
#define ICE_SSL_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>
#include <IceSSL/OpenSSLPluginIF.h>

#ifndef _WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceSSL
{

class SslEndpoint;

class SslConnector : public IceInternal::Connector
{
public:
    
    virtual IceInternal::TransceiverPtr connect(int);
    virtual std::string toString() const;
    
private:
    
    SslConnector(const OpenSSLPluginIPtr&, const std::string&, int);
    virtual ~SslConnector();
    friend class SslEndpoint;

    OpenSSLPluginIPtr _plugin;
    struct sockaddr_in _addr;
};

}

#endif
