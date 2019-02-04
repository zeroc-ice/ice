//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.acm;

import test.Ice.acm.Test.TestIntfPrx;
import test.Ice.acm.Test.TestIntfPrxHelper;
import test.Ice.acm.Test._RemoteObjectAdapterDisp;

public class RemoteObjectAdapterI extends _RemoteObjectAdapterDisp
{
    public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(), Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    public TestIntfPrx getTestIntf(Ice.Current current)
    {
        return _testIntf;
    }

    public void activate(Ice.Current current)
    {
        _adapter.activate();
    }

    public void hold(Ice.Current current)
    {
        _adapter.hold();
    }

    public void deactivate(Ice.Current current)
    {
        try
        {
            _adapter.destroy();
        }
        catch(Ice.ObjectAdapterDeactivatedException ex)
        {
        }
    }

    private Ice.ObjectAdapter _adapter;
    private TestIntfPrx _testIntf;
};
