// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <TestI.h>
#include <TestHelper.h>

using namespace std;
using namespace Ice;
using namespace Test;

void
TestI::transient(const Current& current)
{
    CommunicatorPtr communicator = current.adapter->getCommunicator();
    ObjectAdapterPtr adapter =
        communicator->createObjectAdapterWithEndpoints("TransientTestAdapter",
                                                       TestHelper::getTestEndpoint(communicator->getProperties(), 1));
    adapter->activate();
    adapter->destroy();
}

void
TestI::deactivate(const Current& current)
{
    current.adapter->deactivate();
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
}

string
CookieI::message() const
{
    return "blahblah";
}
