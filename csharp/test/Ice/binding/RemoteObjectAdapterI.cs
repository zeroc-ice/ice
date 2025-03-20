// Copyright (c) ZeroC, Inc.

namespace Ice.binding;

public class RemoteObjectAdapterI : Test.RemoteObjectAdapterDisp_
{
    public RemoteObjectAdapterI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
        _testIntf = Test.TestIntfPrxHelper.uncheckedCast(_adapter.add(new TestI(),
                                                    Ice.Util.stringToIdentity("test")));
        _adapter.activate();
    }

    public override Test.TestIntfPrx
    getTestIntf(Ice.Current current) => _testIntf;

    public override void
    deactivate(Ice.Current current) => _adapter.destroy();

    private readonly Ice.ObjectAdapter _adapter;
    private readonly Test.TestIntfPrx _testIntf;
}
