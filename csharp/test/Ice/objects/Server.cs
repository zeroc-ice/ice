// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using Test;

namespace Ice.objects
{
    public class Server : TestHelper
    {
        public override void run(string[] args)
        {
            var initData = new InitializationData();
            initData.properties = createTestProperties(ref args);
            initData.properties.setProperty("Ice.Warn.Dispatch", "0");
            using (var communicator = initialize(initData))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.Object @object = new InitialI(adapter);
                adapter.add(@object, Ice.Util.stringToIdentity("initial"));
                @object = new F2I();
                adapter.add(@object, Ice.Util.stringToIdentity("F21"));
                @object = new UnexpectedObjectExceptionTestI();
                adapter.add(@object, Ice.Util.stringToIdentity("uoet"));
                adapter.activate();
                serverReady();
                communicator.waitForShutdown();
            }
        }

        public static Task<int> Main(string[] args) =>
            TestDriver.runTestAsync<Server>(args);
    }
}
