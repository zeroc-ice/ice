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

class TcpEndpoint;

class TcpConnector : public Connector
{
public:
    
    virtual Transceiver_ptr connect(int);
    virtual std::string toString() const;
    
private:
    
    TcpConnector(Instance_ptr, const std::string&, int);
    virtual ~TcpConnector();
    friend class TcpEndpoint;

    Instance_ptr instance_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels_ptr traceLevels_;
    ::Ice::Logger_ptr logger_;
#endif
};

}

#endif
