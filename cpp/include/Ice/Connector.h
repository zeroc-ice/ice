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

namespace __Ice
{

class ConnectorI : public Shared
{
public:
    
    virtual Transceiver connect(int) = 0;
    virtual std::string toString() const = 0;
    
protected:

    ConnectorI() { }
    virtual ~ConnectorI() { }
};

}

#endif
