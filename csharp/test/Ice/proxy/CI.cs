// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.proxy;

public sealed class CI : Test.CDisp_, Ice.Object
{
    public CI()
    {
    }

    public override Test.APrx? opA(Test.APrx? a, Ice.Current current) => a;
    public override Test.BPrx? opB(Test.BPrx? b, Ice.Current current) => b;
    public override Test.CPrx? opC(Test.CPrx? c, Ice.Current current) => c;
    public override Test.S opS(Test.S s, Ice.Current current) => s;
}
