//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;
using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.SessionHelper
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Default.Protocol"] = "ice1";
            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("DeactivatedAdapter.Endpoints", GetTestEndpoint(1));
            communicator.CreateObjectAdapter("DeactivatedAdapter");

            communicator.SetProperty("CallbackAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("CallbackAdapter");
            var callbackI = new Callback();
            adapter.Add("callback", callbackI);
            adapter.Activate();
            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
