// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice.middleware;

public sealed class MyObjectI : Test.MyObjectDisp_
{
    public override string getName(Ice.Current current) => "Foo";
}
