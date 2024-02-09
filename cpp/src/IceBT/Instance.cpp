//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceBT/Instance.h>
#include <IceBT/Engine.h>

using namespace std;
using namespace Ice;
using namespace IceBT;

IceBT::Instance::Instance(const EnginePtr& engine, Short type, const string& protocol) :
    ProtocolInstance(engine->communicator(), type, protocol, type == BTSEndpointType),
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
