// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class RemoteObjectAdapterI extends _RemoteObjectAdapterDisp
{
    public
    RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(), 
                                                                _adapter.getCommunicator().stringToIdentity("test")));
        _adapter.activate();
    }

    public TestIntfPrx
    getTestIntf(Ice.Current current)
    {
        return _testIntf;
    }

    public void
    deactivate(Ice.Current current)
    {
        try
        {
            _adapter.destroy();
        }
        catch(Ice.ObjectAdapterDeactivatedException ex)
        {
        }
    }

    final Ice.ObjectAdapter _adapter;
    final TestIntfPrx _testIntf;
};
