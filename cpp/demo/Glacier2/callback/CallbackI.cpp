// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <CallbackI.h>
#include <Ice/Ice.h>

using namespace std;
using namespace Demo;

void
CallbackReceiverI::callback(const Ice::Current&)
{
    cout << "received callback" << endl;
}

void
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Ice::Current& current)
{
    cout << "initiating callback to: " << current.adapter->getCommunicator()->proxyToString(proxy) << endl;
    try
    {
	proxy->callback(current.ctx);
    }
    catch(const Ice::Exception& ex)
    {
	cout << ex << endl;
    }
}

void
CallbackI::shutdown(const Ice::Current& c)
{
    cout << "shutting down..." << endl;
    try
    {
	c.adapter->getCommunicator()->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
	cout << ex << endl;
    }
}
