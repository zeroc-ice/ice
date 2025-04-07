// Copyright (c) ZeroC, Inc.

package test.Ice.binding;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Util;

import test.Ice.binding.Test.RemoteObjectAdapter;
import test.Ice.binding.Test.TestIntfPrx;

public class RemoteObjectAdapterI implements RemoteObjectAdapter {
    public RemoteObjectAdapterI(ObjectAdapter adapter) {
        _adapter = adapter;
        _testIntf =
                TestIntfPrx.uncheckedCast(
                        _adapter.add(new TestI(), Util.stringToIdentity("test")));
        _adapter.activate();
    }

    @Override
    public TestIntfPrx getTestIntf(Current current) {
        return _testIntf;
    }

    @Override
    public void deactivate(Current current) {
        _adapter.destroy();
    }

    final ObjectAdapter _adapter;
    final TestIntfPrx _testIntf;
}
