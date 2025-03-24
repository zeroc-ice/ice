// Copyright (c) ZeroC, Inc.

namespace Ice.inheritance;

public sealed class ICI : Test.MA.ICDisp_
{
    public ICI()
    {
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current) => p;

    public override Test.MA.ICPrx icop(Test.MA.ICPrx p, Ice.Current current) => p;

    public override Test.MB.IB1Prx ib1op(Test.MB.IB1Prx p, Ice.Current current) => p;

    public override Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current) => p;
}
