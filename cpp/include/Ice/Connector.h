// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTOR_H
#define ICE_CONNECTOR_H

#include <Ice/ConnectorF.h>
#include <Ice/InstanceF.h>
#include <Ice/TransceiverF.h>
#include <Ice/Shared.h>

#ifndef WIN32
#   include <netinet/in.h> // For struct sockaddr_in
#endif

namespace __Ice
{

class EmitterFactoryI;

class ConnectorI : public Shared
{
public:
    
    Transceiver connect();
    std::string toString() const;
    
private:

    ConnectorI(const ConnectorI&);
    void operator=(const ConnectorI&);

    ConnectorI(Instance, const std::string&, int);
    virtual ~ConnectorI();
    friend class EmitterFactoryI; // May create ConnectorIs

    Instance instance_;
    struct sockaddr_in addr_;
};

}

#endif
