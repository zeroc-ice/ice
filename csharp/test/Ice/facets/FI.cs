// Copyright (c) ZeroC, Inc.

namespace Ice.facets;

public sealed class FI : Test.FDisp_
{
    public FI()
    {
    }

    public override string callE(Ice.Current current) => "E";

    public override string callF(Ice.Current current) => "F";
}
