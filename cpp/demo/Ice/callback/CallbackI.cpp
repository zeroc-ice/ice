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
using namespace Demo;

void
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Current& current)
{
    cout << "initiating callback" << endl;
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
CallbackI::shutdown(const Current& c)
{
    cout << "shutting down..." << endl;
    try
    {
	c.adapter->getCommunicator()->shutdown();
    }
    catch(const Exception& ex)
    {
	cout << ex << endl;
    }
}
