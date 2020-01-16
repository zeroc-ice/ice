//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class TestIntf : ITestIntf
{
    public TestIntf(string[] args) => _args = args;

    public string
    getProperty(string name, Ice.Current current) => current.Adapter.Communicator.GetProperty(name) ?? "";

    public string[]
    getArgs(Ice.Current current) => _args;

    private string[] _args;
}
