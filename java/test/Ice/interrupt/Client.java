// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

public class Client extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        try
        {
            AllTests.allTests(this, communicator(), getWriter());
        }
        catch (InterruptedException e)
        {
            e.printStackTrace();
            throw new RuntimeException();
        }
        return 0;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.interrupt");
        //
        // We need to enable the background IO so that Ice is interrupt
        // safe for this test.
        //
        initData.properties.setProperty("Ice.BackgroundIO", "1");
        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        initData.properties.setProperty("Ice.MessageSizeMax", "20000");
        return initData;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        int result = app.main("Client", args);
        System.gc();
        System.exit(result);
    }
}
