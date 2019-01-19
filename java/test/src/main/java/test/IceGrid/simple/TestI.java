//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceGrid.simple;

import test.IceGrid.simple.Test.TestIntf;

public class TestI implements TestIntf
{
    public TestI()
    {
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
