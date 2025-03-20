// Copyright (c) ZeroC, Inc.

namespace Ice.inheritance;

public sealed class InitialI : Test.InitialDisp_
{
    public InitialI(Ice.ObjectAdapter adapter)
    {
        _ia = Test.MA.IAPrxHelper.uncheckedCast(adapter.addWithUUID(new IAI()));
        _ib1 = Test.MB.IB1PrxHelper.uncheckedCast(adapter.addWithUUID(new IB1I()));
        _ib2 = Test.MB.IB2PrxHelper.uncheckedCast(adapter.addWithUUID(new IB2I()));
        _ic = Test.MA.ICPrxHelper.uncheckedCast(adapter.addWithUUID(new ICI()));
    }

    public override Test.MA.IAPrx iaop(Ice.Current current) => _ia;

    public override Test.MB.IB1Prx ib1op(Ice.Current current) => _ib1;

    public override Test.MB.IB2Prx ib2op(Ice.Current current) => _ib2;

    public override Test.MA.ICPrx icop(Ice.Current current) => _ic;

    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    private readonly Test.MA.IAPrx _ia;
    private readonly Test.MB.IB1Prx _ib1;
    private readonly Test.MB.IB2Prx _ib2;
    private readonly Test.MA.ICPrx _ic;
}
