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
#include <Ice/TransceiverF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ConnectorI : public Shared
{
public:
    
    Transceiver connect();
    
private:

    ConnectorI(const ConnectorI&);
    void operator=(const ConnectorI&);

    ConnectorI(const std::string&, int);
    virtual ~ConnectorI();
    void destroy();
    friend class xxxI; // May create and destroy ConnectorIs

    std::string host_;
    int port_;
};

}

#endif
