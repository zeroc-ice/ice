//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Linq;
using System.Collections.Generic;

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        Dictionary<string, string> properties = CreateTestProperties(ref args);
        properties["Ice.Warn.Connections"] = "0";
        using Ice.Communicator communicator = Initialize(properties);
        var ports = args.Select(v => int.Parse(v)).ToList();
        if (ports.Count == 0)
        {
            throw new ArgumentException("Client: no ports specified");
        }
        AllTests.allTests(this, ports);
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
