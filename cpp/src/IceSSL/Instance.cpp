// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Config.h>
#ifdef _WIN32
#   include <winsock2.h>
#endif
#include <IceSSL/Instance.h>
#include <Ice/Properties.h>

using namespace std;
using namespace Ice;
using namespace IceSSL;

IceUtil::Shared* IceSSL::upCast(IceSSL::Instance* p) { return p; }

IceSSL::Instance::Instance(const SSLEnginePtr& engine, Short type, const string& protocol) :
    ProtocolInstance(engine->communicator(), type, protocol),
    _engine(engine)
{
    _securityTraceLevel = properties()->getPropertyAsInt("IceSSL.Trace.Security");
    _securityTraceCategory = "Security";
}

IceSSL::Instance::~Instance()
{
}

bool
IceSSL::Instance::initialized() const
{
    return _engine->initialized();
}

int
IceSSL::Instance::securityTraceLevel() const
{
    return _securityTraceLevel;
}

string
IceSSL::Instance::securityTraceCategory() const
{
    return _securityTraceCategory;
}
