// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
