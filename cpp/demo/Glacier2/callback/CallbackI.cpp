// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;

void
CallbackReceiverI::callback(const Current&)
{
    cout << "received callback" << endl;
}

void
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Current& current)
{
    cout << "initiating callback to: " << current.adapter->getCommunicator()->proxyToString(proxy) << endl;
    try
    {
	proxy->callback(current.ctx);
    }
    catch(const Exception& ex)
    {
	cout << ex << endl;
    }
}

void
CallbackI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    try
    {
	c.adapter->getCommunicator()->shutdown();
    }
    catch(const Exception& ex)
    {
	cout << ex << endl;
    }
}
