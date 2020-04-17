//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

using Test;

public class TestIntf : ITestIntf
{
    private string[] _args;

    public TestIntf(string[] args) => _args = args;

    public string getProperty(string name, Ice.Current current) => current.Adapter.Communicator.GetProperty(name) ?? "";

    public IEnumerable<string> getArgs(Ice.Current current) => _args;
}
