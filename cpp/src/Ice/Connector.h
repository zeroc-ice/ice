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

#include <IceUtil/Shared.h>
#include <Ice/ConnectorF.h>
#include <Ice/TransceiverF.h>

namespace IceInternal
{

class ICE_PROTOCOL_API Connector : public ::IceUtil::Shared
{
public:
    
    virtual TransceiverPtr connect(int) = 0;
    virtual std::string toString() const = 0;
};

}

#endif
