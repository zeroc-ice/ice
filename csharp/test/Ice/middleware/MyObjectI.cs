// Copyright (c) ZeroC, Inc.

namespace Ice.middleware;

public sealed class MyObjectI : Test.MyObjectDisp_
{
    public override string getName(Ice.Current current) => "Foo";
}
