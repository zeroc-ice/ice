// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTOR_H
#define ICE_SSL_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>
#include <IceSSL/PluginBaseIF.h>

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
    
    SslConnector(const PluginBaseIPtr&, const std::string&, int);
    virtual ~SslConnector();
    friend class SslEndpoint;

    PluginBaseIPtr _plugin;
    struct sockaddr_in _addr;
};

}

#endif
