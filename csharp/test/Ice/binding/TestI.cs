// Copyright (c) ZeroC, Inc.

namespace Ice.binding;

public class TestI : Test.TestIntfDisp_
{
    public override string getAdapterName(Ice.Current current)
    {
        return current.adapter.getName();
    }
}
