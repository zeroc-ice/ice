// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.echo;

public class Server extends test.Util.Application
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

    @Override
    public int run(String[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        BlobjectI blob = new BlobjectI();
        adapter.addDefaultServant(blob, "");
        adapter.add(new EchoI(blob), com.zeroc.Ice.Util.stringToIdentity("__echo"));
        adapter.activate();
        return WAIT;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.echo");
        return initData;
    }

    public static void main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
