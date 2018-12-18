// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import test.Ice.binding.Test.TestIntfPrx;
import test.Ice.binding.Test.RemoteObjectAdapter;

public class RemoteObjectAdapterI implements RemoteObjectAdapter
{
    public RemoteObjectAdapterI(com.zeroc.Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrx.uncheckedCast(_adapter.add(new TestI(),
                                                           com.zeroc.Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    @Override
    public TestIntfPrx getTestIntf(com.zeroc.Ice.Current current)
    {
        return _testIntf;
    }

    @Override
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

    final com.zeroc.Ice.ObjectAdapter _adapter;
    final TestIntfPrx _testIntf;
}
