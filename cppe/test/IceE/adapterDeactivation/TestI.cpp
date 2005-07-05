// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <TestI.h>

using namespace std;
using namespace IceE;

void
TestI::transient(const Current& current)
{
    CommunicatorPtr communicator = current.adapter->getCommunicator();
    
    ObjectAdapterPtr adapter =
	communicator->createObjectAdapterWithEndpoints("TransientTestAdapter", "default -p 9999");
    adapter->activate();
    adapter->deactivate();
    adapter->waitForDeactivate();
}

void
TestI::deactivate(const Current& current)
{
    current.adapter->deactivate();
    IceE::ThreadControl::sleep(IceE::Time::seconds(1));
}

string
CookieI::message() const
{
    return "blahblah";
}
