// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WS_INSTANCE_H
#define ICE_WS_INSTANCE_H

#include <IceWS/InstanceF.h>
#include <Ice/ProtocolInstance.h>
#include <Ice/CommunicatorF.h>

namespace IceWS
{

class Instance : public IceInternal::ProtocolInstance
{
public:

    Instance(const Ice::CommunicatorPtr&, Ice::Short, const std::string&);
    virtual ~Instance();
};

}

#endif
