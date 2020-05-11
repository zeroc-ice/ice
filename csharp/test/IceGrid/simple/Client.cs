//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args,
            new Dictionary<string, string> { ["Ice.Default.Encoding"] = "1.1" });
        if (args.Any(v => v.Equals("--with-deploy")))
        {
            AllTests.allTestsWithDeploy(this);
        }
        else
        {
            AllTests.allTests(this);
        }
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
