// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Linq;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.IceGrid.Test.Simple
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            if (args.Any(v => v.Equals("--with-deploy")))
            {
                await AllTests.RunWithDeployAsync(this);
            }
            else
            {
                await AllTests.RunAsync(this);
            }
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
