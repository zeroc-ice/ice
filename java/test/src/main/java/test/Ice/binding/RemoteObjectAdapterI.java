// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import test.Ice.binding.Test.RemoteObjectAdapter;
import test.Ice.binding.Test.TestIntfPrx;

public class RemoteObjectAdapterI implements RemoteObjectAdapter {
    public RemoteObjectAdapterI(com.zeroc.Ice.ObjectAdapter adapter) {
        _adapter = adapter;
        _testIntf =
                TestIntfPrx.uncheckedCast(
                        _adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    @Override
    public TestIntfPrx getTestIntf(com.zeroc.Ice.Current current) {
        return _testIntf;
    }

    @Override
    public void deactivate(com.zeroc.Ice.Current current) {
        _adapter.destroy();
    }

    final com.zeroc.Ice.ObjectAdapter _adapter;
    final TestIntfPrx _testIntf;
}
