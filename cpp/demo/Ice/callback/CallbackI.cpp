// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

CallbackI::CallbackI(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Current& current)
{
    cout << "initiating callback" << endl;
    proxy->callback(current.ctx);
}

void
CallbackI::shutdown(const Ice::Current&)
{
    cout << "Shutting down..." << endl;
    _communicator->shutdown();
}
