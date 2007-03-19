// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <CallbackI.h>

using namespace std;
using namespace Demo;

void
CallbackReceiverI::callback(const Ice::Current&)
{
#ifdef __xlC__

        //
        // The xlC compiler synchronizes cin and cout; to see the messages
        // while accepting input through cin, we have to print the messages
        // with printf
        //

        printf("received callback\n");
        fflush(0);
#else
    cout << "received callback" << endl;
#endif
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
