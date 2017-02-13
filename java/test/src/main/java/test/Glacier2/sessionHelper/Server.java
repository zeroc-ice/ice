// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.sessionHelper;

public class Server extends test.Util.Application
{
    public int
    run(String[] args)
    {
        communicator().getProperties().setProperty("DeactivatedAdapter.Endpoints", "default -p 12011");
        communicator().createObjectAdapter("DeactivatedAdapter");

        communicator().getProperties().setProperty("CallbackAdapter.Endpoints", "default -p 12010");
        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("CallbackAdapter");
        adapter.add(new CallbackI(), communicator().stringToIdentity("callback"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }


    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Glacier2.router");

        return initData;
    }

    public static void
    main(String[] args)
    {
        Server c = new Server();
        int status = c.main("Server", args);

        System.gc();
        System.exit(status);
    }
}
