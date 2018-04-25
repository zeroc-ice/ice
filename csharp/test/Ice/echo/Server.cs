// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
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

    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        BlobjectI blob = new BlobjectI();
        adapter.addDefaultServant(blob, "");
        adapter.add(new EchoI(blob), Ice.Util.stringToIdentity("__echo"));
        adapter.activate();

        communicator().waitForShutdown();
        return 0;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
