// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class ICE_API Connector : public ::IceUtil::Shared
{
public:
    
    virtual TransceiverPtr connect() = 0;

    virtual Ice::Short type() const = 0;
    virtual std::string toString() const = 0;

    virtual bool operator==(const Connector&) const = 0;
    virtual bool operator!=(const Connector&) const = 0;
    virtual bool operator<(const Connector&) const = 0;
};

}

#endif
