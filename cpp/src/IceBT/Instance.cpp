// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceBT/Instance.h>
#include <IceBT/Engine.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceUtil::Shared* IceBT::upCast(IceBT::Instance* p) { return p; }

IceBT::Instance::Instance(const EnginePtr& engine, Short type, const string& protocol) :
    ProtocolInstance(engine->communicator(), type, protocol, true),
    _engine(engine)
{
}

IceBT::Instance::~Instance()
{
}

bool
IceBT::Instance::initialized() const
{
    return _engine->initialized();
}
