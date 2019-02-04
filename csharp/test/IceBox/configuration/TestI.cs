//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class TestI : TestIntfDisp_
{
    public TestI(string[] args)
    {
        _args = args;
    }

    public override string
    getProperty(string name, Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(name);
    }

    public override string[]
    getArgs(Ice.Current current)
    {
        return _args;
    }

    private string[] _args;
}
