// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class CallbackServer extends Ice.Application
{
    public int
    run(String[] args)
    {
	communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "tcp -p 12345 -t 10000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        CallbackPrx self = CallbackPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("callback")));
        adapter.add(new CallbackI(communicator()), Ice.Util.stringToIdentity("callback"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }
}
