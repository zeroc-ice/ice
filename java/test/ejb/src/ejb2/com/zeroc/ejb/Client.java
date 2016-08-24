// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import javax.naming.*;

import com.zeroc.ice.Test.*;

public class Client extends com.zeroc.Ice.Application
{
    public int run(String[] args)
    {
        System.out.print("initializing EJBs... ");
        System.out.flush();
        try
        {
            // Initialize the EJBs
            InitialContext ic = new InitialContext();
            Init init = (Init)ic.lookup("InitEJB2Service");
            init.init();
        }
        catch(Exception ex)
        {
            System.err.println("couldn't resolve init bean:\n" + ex);
            return 1;
        }
        System.out.println("ok");

        System.out.print("setting and getting account... ");
        System.out.flush();
        String str = "ejb2/service:tcp -h localhost -p 10000";
        ServicePrx proxy = ServicePrx.checkedCast(communicator().stringToProxy(str));
        proxy.addAccount(new Account("id1", "foo"));
        Account s = proxy.getAccount("id1");
        if(!s.id.equals("id1") || !s.foo.equals("foo"))
        {
            throw new RuntimeException("invalid value: " + s.id);
        }

        s = proxy.getAccount("id2");
        if(!s.id.equals("id2"))
        {
            throw new RuntimeException("invalid value: " + s.id);
        }

        System.out.println("ok");
        return 0;
    }

    public static void main(String[] args)
    {
        Client app = new Client();
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = com.zeroc.Ice.Util.createProperties();
        initData.properties.setProperty("Ice.Default.Package", "com.zeroc.ice");
        int status = app.main("Client", args, initData);
        System.exit(status);
    }
}
