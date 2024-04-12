//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "SSLInstance.h"
#include "SSLEngine.h"

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceSSL::Instance::Instance(const SSLEnginePtr& engine, int16_t type, const string& protocol)
    : ProtocolInstance(engine->instance(), type, protocol, true),
      _engine(engine)
{
}
