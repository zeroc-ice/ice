// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class RemoteObjectAdapterI : RemoteObjectAdapterDisp_
{
    public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(), 
                                                    Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    public override TestIntfPrx
    getTestIntf(Ice.Current current)
    {
        return _testIntf;
    }

    public override void
    deactivate(Ice.Current current)
    {
        try
        {
            _adapter.destroy();
        }
        catch(Ice.ObjectAdapterDeactivatedException)
        {
        }
    }

    private Ice.ObjectAdapter _adapter;
    private TestIntfPrx _testIntf;
};
