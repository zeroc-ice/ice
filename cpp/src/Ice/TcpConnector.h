// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_TCP_CONNECTOR_H
#define ICE_TCP_CONNECTOR_H

#include <Ice/TransceiverF.h>
#include <Ice/InstanceF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/LoggerF.h>
#include <Ice/Connector.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace IceInternal
{

class TcpEndpoint;

class TcpConnector : public Connector
{
public:
    
    virtual TransceiverPtr connect(int);
    virtual std::string toString() const;
    
private:
    
    TcpConnector(const InstancePtr&, const std::string&, int);
    virtual ~TcpConnector();
    friend class TcpEndpoint;

    InstancePtr _instance;
    struct sockaddr_in _addr;
#ifndef ICE_NO_TRACE
    TraceLevelsPtr _traceLevels;
    ::Ice::LoggerPtr _logger;
#endif
};

}

#endif
