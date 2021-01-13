// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.AdapterId", "test");
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            var prx = adapter.Add("test", new MyDerivedClass(), IMyDerivedClassPrx.Factory);
            // Don't activate OA to ensure collocation is used.

            await AllTests.RunAsync(this);
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Collocated>(communicator, args);
        }
    }
}
