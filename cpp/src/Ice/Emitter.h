// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_EMITTER_H
#define ICE_EMITTER_H

#include <Ice/EmitterF.h>
#include <Ice/ConnectionF.h>
#include <Ice/InstanceF.h>
#include <Ice/EndpointF.h>
#include <IceUtil/Shared.h>

namespace IceInternal
{

class EmitterFactory : public ::IceUtil::Shared, public JTCMutex
{
public:

    EmitterFactory(const InstancePtr&);
    virtual ~EmitterFactory();

    void destroy();
    ConnectionPtr create(const std::vector<EndpointPtr>&);

private:

    InstancePtr _instance;
    std::map<EndpointPtr, ConnectionPtr> _connections;
};

}

#endif
