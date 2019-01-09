// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <IceSSL/Instance.h>
#include <IceSSL/SSLEngine.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(IceSSL::Instance* p) { return p; }

IceSSL::Instance::Instance(const SSLEnginePtr& engine, Short type, const string& protocol) :
    ProtocolInstance(engine->communicator(), type, protocol, true),
    _engine(engine)
{
}

IceSSL::Instance::~Instance()
{
}

bool
IceSSL::Instance::initialized() const
{
    return _engine->initialized();
}
