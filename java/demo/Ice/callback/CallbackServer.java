// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
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
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Callback.Server");
        CallbackPrx self = CallbackPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("callback")));
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("callback"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }
}
