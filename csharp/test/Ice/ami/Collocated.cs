//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMI
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);

            properties["Ice.Warn.AMICallback"] = "0";
            //
            // Limit the send buffer size, this test relies on the socket
            // send() blocking after sending a given amount of data.
            //
            properties["Ice.TCP.SndSize"] = "50K";

            await using Communicator communicator = Initialize(properties);

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));

            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            //adapter.activate(); // Collocated test doesn't need to activate the OA

            ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter2", serializeDispatch: true);
            adapter2.Add("serialized", new TestIntf());
            //adapter2.Activate();

            AllTests.allTests(this, true);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Collocated>(args);
    }
}
