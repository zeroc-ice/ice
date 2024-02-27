//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceSSL::Instance::Instance(const SSLEnginePtr& engine, int16_t type, const string& protocol) :
    ProtocolInstance(engine->communicator(), type, protocol, true),
    _engine(engine)
{
}

bool
IceSSL::Instance::initialized() const
{
    return _engine->initialized();
}
