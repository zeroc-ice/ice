// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class CallbackServer extends Ice.Application
{
    public int
    run(String[] args)
    {
	communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "tcp -p 12345 -t 2000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        CallbackPrx self = CallbackPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("callback")));
        adapter.add(new CallbackI(communicator()), Ice.Util.stringToIdentity("callback"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }
}
