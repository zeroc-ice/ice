// Copyright (c) ZeroC, Inc.

namespace Ice.inheritance;

public sealed class IAI : Test.MA.IADisp_
{
    public IAI()
    {
    }

    public override Test.MA.IAPrx iaop(Test.MA.IAPrx p, Ice.Current current) => p;
}
