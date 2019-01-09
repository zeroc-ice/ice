// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.echo;

public class Server extends test.TestHelper
{
    class EchoI implements test.Ice.echo.Test.Echo
    {
        public EchoI(BlobjectI blob)
        {
            _blob = blob;
        }

        @Override
        public void startBatch(com.zeroc.Ice.Current current)
        {
            _blob.startBatch();
        }

        @Override
        public void flushBatch(com.zeroc.Ice.Current current)
        {
            _blob.flushBatch();
        }

        @Override
        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }

        final private BlobjectI _blob;
    }

    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.echo");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            BlobjectI blob = new BlobjectI();
            adapter.addDefaultServant(blob, "");
            adapter.add(new EchoI(blob), com.zeroc.Ice.Util.stringToIdentity("__echo"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
