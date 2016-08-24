// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import test.Ice.binding.Test.TestIntfPrx;
import test.Ice.binding.Test.TestIntfPrxHelper;
import test.Ice.binding.Test._RemoteObjectAdapterDisp;

public class RemoteObjectAdapterI extends _RemoteObjectAdapterDisp
{
    public
    RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(), Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    @Override
    public TestIntfPrx
    getTestIntf(Ice.Current current)
    {
        return _testIntf;
    }

    @Override
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
