// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTOR_H
#define ICE_SSL_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
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
    
    SslConnector(const IceInternal::InstancePtr&, const std::string&, int);
    virtual ~SslConnector();
    friend class SslEndpoint;

    IceInternal::InstancePtr _instance;
    IceInternal::TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
    struct sockaddr_in _addr;
};

}

#endif
