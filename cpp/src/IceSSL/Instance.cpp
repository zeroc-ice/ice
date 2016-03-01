// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
#include <IceSSL/SSLEngine.h>
#include <Ice/Properties.h>

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
