// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UDP_CONNECTOR_H
#define ICE_UDP_CONNECTOR_H

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

class EmitterFactoryI;

class UdpConnectorI : public ConnectorI
{
public:
    
    virtual Transceiver initialize();
    virtual Transceiver connect(int);
    virtual std::string toString() const;
    
private:

    UdpConnectorI(const UdpConnectorI&);
    void operator=(const UdpConnectorI&);

    UdpConnectorI(Instance, const std::string&, int);
    virtual ~UdpConnectorI();
    friend class EmitterFactoryI; // May create UdpConnectorIs

    Instance instance_;
    struct sockaddr_in addr_;
#ifndef ICE_NO_TRACE
    TraceLevels traceLevels_;
    ::Ice::Logger logger_;
#endif
};

}

#endif
