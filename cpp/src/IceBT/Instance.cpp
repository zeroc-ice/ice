// Copyright (c) ZeroC, Inc.

#include "Instance.h"
#include "Engine.h"

using namespace std;
using namespace Ice;
using namespace IceBT;

IceBT::Instance::Instance(const EnginePtr& engine, int16_t type, const string& protocol)
    : ProtocolInstance(engine->communicator(), type, protocol, type == BTSEndpointType),
      _engine(engine)
{
}

IceBT::Instance::~Instance() = default;

EnginePtr
IceBT::Instance::engine() const
{
    EnginePtr engine = _engine.lock();
    if (!engine)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return engine;
}

bool
IceBT::Instance::initialized() const
{
    return engine()->initialized();
}
