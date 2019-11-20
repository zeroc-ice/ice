//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    private class EchoI : Test.Echo
    {
        public EchoI(BlobjectI blob)
        {
            _blob = blob;
        }

        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        private BlobjectI _blob;
    }

    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            BlobjectI blob = new BlobjectI();
            adapter.addDefaultServant((incoming, current) => blob.Dispatch(incoming, current), "");
            adapter.Add(new EchoI(blob), Ice.Util.stringToIdentity("__echo"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return TestDriver.runTest<Server>(args);
    }
}
