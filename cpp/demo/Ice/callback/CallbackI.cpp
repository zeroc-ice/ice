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
    cout << "initiating callback" << endl;
    proxy->callback(current.ctx);
}

void
CallbackI::shutdown(const Ice::Current& c)
{
    cout << "Shutting down..." << endl;
    c.adapter->getCommunicator()->shutdown();
}
