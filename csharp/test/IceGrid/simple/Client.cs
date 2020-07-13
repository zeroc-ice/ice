//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;

namespace ZeroC.IceGrid.Test.Simple
{
    public class Client : TestHelper
    {
        public override Task Run(string[] args)
        {
            using var communicator = Initialize(ref args,
                new Dictionary<string, string> { ["Ice.Default.Protocol"] = "ice1", ["Ice.Default.Encoding"] = "1.1" });
            if (args.Any(v => v.Equals("--with-deploy")))
            {
                AllTests.allTestsWithDeploy(this);
            }
            else
            {
                AllTests.allTests(this);
            }
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
