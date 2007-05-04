// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <CallbackSenderI.h>
#include <IceE/IceE.h>

using namespace std;
using namespace Demo;

void
CallbackSenderI::initiateCallback(const CallbackReceiverPrx& proxy, const Ice::Current& current)
{
    printf("initiating callback\n");
    try
    {
	proxy->callback(current.ctx);
    }
    catch(const Ice::Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
    }
}

void
CallbackSenderI::shutdown(const Ice::Current& c)
{
    printf("shutting down...\n");
    try
    {
	c.adapter->getCommunicator()->shutdown();
    }
    catch(const Ice::Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
    }
}
