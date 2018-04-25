// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import com.zeroc.ice.Test.*;

public class DatabaseServer extends Ice.Application
{
    public int
    run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        Ice.ObjectAdapter adapter = communicator().createObjectAdapterWithEndpoints("DB", "tcp -h localhost -p 10001");
        adapter.add(new DatabaseI(), Ice.Util.stringToIdentity("db"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void
    main(String[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Default.Package", "com.zeroc.ice");

        DatabaseServer app = new DatabaseServer();
        int status = app.main("Server", args, initData);
        System.exit(status);
    }
}
