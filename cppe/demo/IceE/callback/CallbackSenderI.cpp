// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <CallbackSenderI.h>

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
