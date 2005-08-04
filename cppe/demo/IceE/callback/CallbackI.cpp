// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <CallbackI.h>

using namespace std;
using namespace Ice;
using namespace Demo;

void
CallbackI::initiateCallback(const CallbackReceiverPrx& proxy, const Current& current)
{
    printf("initiating callback\n");
    try
    {
	proxy->callback(current.ctx);
    }
    catch(const Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
    }
}

void
CallbackI::shutdown(const Current& c)
{
    printf("shutting down...\n");
    try
    {
	c.adapter->getCommunicator()->shutdown();
    }
    catch(const Exception& ex)
    {
	fprintf(stderr, "%s\n", ex.toString().c_str());
    }
}
