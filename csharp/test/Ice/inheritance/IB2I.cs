// Copyright (c) ZeroC, Inc.

namespace Ice.inheritance;

public sealed class IB2I : Test.MB.IB2Disp_
{
    public IB2I()
    {
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current)
    {
        return p;
    }

    public override Test.MB.IB2Prx ib2op(Test.MB.IB2Prx p, Ice.Current current)
    {
        return p;
    }
}
