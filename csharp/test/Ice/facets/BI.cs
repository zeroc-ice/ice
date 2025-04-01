// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class BI : Test.BDisp_
{
    public BI()
    {
    }

    public override string callA(Ice.Current current) => "A";

    public override string callB(Ice.Current current) => "B";
}
