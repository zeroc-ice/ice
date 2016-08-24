// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import com.zeroc.ice.Test.*;

public class DatabaseServer extends com.zeroc.Ice.Application
{
    public int run(String[] args)
    {
        if(args.length > 0)
        {
            System.err.println(appName() + ": too many arguments");
            return 1;
        }

        com.zeroc.Ice.ObjectAdapter adapter =
            communicator().createObjectAdapterWithEndpoints("DB", "tcp -h localhost -p 10002");
        adapter.add(new DatabaseI(), com.zeroc.Ice.Util.stringToIdentity("db"));
        adapter.activate();
        communicator().waitForShutdown();
        return 0;
    }

    public static void main(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Default.Package", "com.zeroc.ice");

        DatabaseServer app = new DatabaseServer();
        int status = app.main("Server", args, initData);
        System.exit(status);
    }
}
