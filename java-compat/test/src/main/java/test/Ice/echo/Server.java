// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.echo;

public class Server extends test.TestHelper
{
    class EchoI extends test.Ice.echo.Test._EchoDisp
    {
        public EchoI(BlobjectI blob)
        {
            _blob = blob;
        }

        @Override
        public void startBatch(Ice.Current current)
        {
            _blob.startBatch();
        }

        @Override
        public void flushBatch(Ice.Current current)
        {
            _blob.flushBatch();
        }

        @Override
        public void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        final private BlobjectI _blob;
    };

    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.echo");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            BlobjectI blob = new BlobjectI();
            adapter.addDefaultServant(blob, "");
            adapter.add(new EchoI(blob), Ice.Util.stringToIdentity("__echo"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
