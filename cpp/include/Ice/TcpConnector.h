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

namespace __Ice
{

class TcpEndpointI;

class TcpConnectorI : public ConnectorI
{
public:
    
    virtual Transceiver connect(int);
    virtual std::string toString() const;
    
private:
    
    TcpConnectorI(Instance, const std::string&, int);
    virtual ~TcpConnectorI();
    friend class TcpEndpointI;

    Instance instance_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif
