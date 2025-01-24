// Copyright (c) ZeroC, Inc.

#include "SSLInstance.h"
#include "SSLEngine.h"

using namespace std;
using namespace Ice;
using namespace Ice::SSL;

Ice::SSL::Instance::Instance(const SSLEnginePtr& engine, int16_t type, const string& protocol)
    : ProtocolInstance(engine->instance(), type, protocol, true),
      _engine(engine)
{
}

Ice::SSL::Instance::~Instance() = default; // avoid weak vtable

SSLEnginePtr
Ice::SSL::Instance::engine() const
{
    SSLEnginePtr engine = _engine.lock();
    if (!engine)
    {
        throw CommunicatorDestroyedException{__FILE__, __LINE__};
    }
    return engine;
}
