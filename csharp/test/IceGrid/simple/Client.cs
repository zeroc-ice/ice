//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Linq;

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
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
