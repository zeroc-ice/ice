// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

class NestedServer : Ice.Application
{
    public override int run(string[] args)
    {
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("Nested.Server");
        NestedPrx self = NestedPrxHelper.uncheckedCast(adapter.createProxy(Ice.Util.stringToIdentity("nestedServer")));
        adapter.add(new NestedI(self), Ice.Util.stringToIdentity("nestedServer"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }
}
