// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
