// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTOR_I_H
#define ICE_SSL_CONNECTOR_I_H

#include <Ice/LoggerF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Connector.h>
#include <IceSSL/InstanceF.h>

#ifdef _WIN32
#   include <winsock2.h>
#else
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceSSL
{

class EndpointI;

class ConnectorI : public IceInternal::Connector
{
public:
    
    virtual IceInternal::TransceiverPtr connect(int);
    virtual std::string toString() const;
    
private:
    
    ConnectorI(const InstancePtr&, const std::string&, int);
    virtual ~ConnectorI();
    friend class EndpointI;

    InstancePtr _instance;
    std::string _host;
    Ice::LoggerPtr _logger;
    struct sockaddr_in _addr;
};

}

#endif
