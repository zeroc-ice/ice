// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

import test.Ice.acm.Test.TestIntfPrx;
import test.Ice.acm.Test.RemoteObjectAdapter;

public class RemoteObjectAdapterI implements RemoteObjectAdapter
{
    public RemoteObjectAdapterI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrx.uncheckedCast(_adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    public TestIntfPrx getTestIntf(com.zeroc.Ice.Current current)
    {
        return _testIntf;
    }
    
    public void activate(com.zeroc.Ice.Current current)
    {
        _adapter.activate();
    }

    public void hold(com.zeroc.Ice.Current current)
    {
        _adapter.hold();
    }

    public void deactivate(com.zeroc.Ice.Current current)
    {
        try
        {
            _adapter.destroy();
        }
        catch(com.zeroc.Ice.ObjectAdapterDeactivatedException ex)
        {
        }
    }

    private com.zeroc.Ice.ObjectAdapter _adapter;
    private TestIntfPrx _testIntf;
}
