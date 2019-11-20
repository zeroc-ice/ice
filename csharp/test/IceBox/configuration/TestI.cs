//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class TestI : TestIntf
{
    public TestI(string[] args)
    {
        _args = args;
    }

    public string
    getProperty(string name, Ice.Current current)
    {
        return current.adapter.GetCommunicator().getProperties().getProperty(name);
    }

    public string[]
    getArgs(Ice.Current current)
    {
        return _args;
    }

    private string[] _args;
}
