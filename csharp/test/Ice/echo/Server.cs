// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    private class EchoI : Test.EchoDisp_
    {
        public EchoI(BlobjectI blob)
        {
            _blob = blob;
        }

        public override void startBatch(Ice.Current current)
        {
            _blob.startBatch();
        }

        public override void flushBatch(Ice.Current current)
        {
            _blob.flushBatch();
        }

        public override void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        private BlobjectI _blob;
    }

    public override void run(string[] args)
    {
        using(var communicator = initialize(ref args))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            BlobjectI blob = new BlobjectI();
            adapter.addDefaultServant(blob, "");
            adapter.add(new EchoI(blob), Ice.Util.stringToIdentity("__echo"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
