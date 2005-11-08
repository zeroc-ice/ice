// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTOR_H
#define ICE_SSL_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/LoggerF.h>
#include <IceSSL/TraceLevelsF.h>
#include <IceSSL/OpenSSLPluginIF.h>
#include <Ice/Connector.h>

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
    friend class SslEndpointI;

    const OpenSSLPluginIPtr _plugin;
    struct sockaddr_in _addr;
    const TraceLevelsPtr _traceLevels;
    const ::Ice::LoggerPtr _logger;
};

}

#endif
