// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

class BackendServer extends Ice.Application
{
    public int
    run(String[] args)
    {
        communicator().getProperties().setProperty("BackendAdapter.Endpoints", "tcp -p 12010 -t 10000");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("BackendAdapter");
        adapter.addServantLocator(new ServantLocatorI(), "");
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }
}
