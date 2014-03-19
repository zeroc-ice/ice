// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceWS/Instance.h>
#include <IceWS/EndpointI.h>
#include <Ice/ProtocolPluginFacade.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceWS;

IceUtil::Shared* IceWS::upCast(IceWS::Instance* p) { return p; }

IceWS::Instance::Instance(const CommunicatorPtr& communicator, Short type, const string& protocol) :
    ProtocolInstance(communicator, type, protocol)
{
}

IceWS::Instance::~Instance()
{
}
