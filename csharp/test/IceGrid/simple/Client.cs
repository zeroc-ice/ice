//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceGrid.Test.Simple
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(
                ref args,
                new Dictionary<string, string>
                {
                    ["Ice.Default.Protocol"] = "ice1",
                    ["Ice.Default.Encoding"] = "1.1"
                });
            if (args.Any(v => v.Equals("--with-deploy")))
            {
                AllTests.RunWithDeploy(this);
            }
            else
            {
                AllTests.Run(this);
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
