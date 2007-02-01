// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

class AttackClient extends Ice.Application
{
    public int
    run(String[] args)
    {
        System.out.print("getting router... ");
        System.out.flush();
        Ice.ObjectPrx routerBase = communicator().stringToProxy("Glacier2/router:default -p 12347 -t 10000");
        Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.checkedCast(routerBase);
        test(router != null);
        System.out.println("ok");

        System.out.print("creating session... ");
        System.out.flush();
        try
        {
            Glacier2.SessionPrx session = router.createSession("userid", "abc123");
        }
        catch(Glacier2.PermissionDeniedException ex)
        {
            assert(false);
        }
        catch(Glacier2.CannotCreateSessionException ex)
        {
            assert(false);
        }
        communicator().setDefaultRouter(router);
        System.out.println("ok");

        System.out.print("making thousands of invocations on proxies... ");
        System.out.flush();
        Ice.ObjectPrx backendBase = communicator().stringToProxy("dummy:tcp -p 12010 -t 10000");
        BackendPrx backend = BackendPrxHelper.uncheckedCast(backendBase);
        backend.ice_ping();

        java.util.HashMap backends = new java.util.HashMap();
        java.util.Random rand = new java.util.Random();

        String msg = "";
        for(int i = 1; i <= 10000; ++i)
        {
            if(i % 100 == 0)
            {
                for(int j = 0; j < msg.length(); ++j)
                {
                    System.out.print('\b');
                }

                msg = "" + i;
                System.out.print(i);
                System.out.flush();
            }

            Ice.Identity ident = new Ice.Identity();

            StringBuffer name = new StringBuffer();
            name.append((char)('A' + rand.nextInt(26)));
            ident.name = name.toString();

            StringBuffer category = new StringBuffer();
            int len = rand.nextInt(2);
            for(int j = 0; j < len; ++j)
            {
                category.append((char)('a' + rand.nextInt(26)));
            }
            ident.category = category.toString();

            BackendPrx newBackend = BackendPrxHelper.uncheckedCast(backendBase.ice_identity(ident));

            if(!backends.containsKey(newBackend))
            {
                backends.put(newBackend, newBackend);
                backend = newBackend;
            }
            else
            {
                backend = (BackendPrx)backends.get(newBackend);
            }

            backend.ice_ping();
        }

        for(int j = 0; j < msg.length(); ++j)
        {
            System.out.print('\b');
        }
        for(int j = 0; j < msg.length(); ++j)
        {
            System.out.print(' ');
        }
        for(int j = 0; j < msg.length(); ++j)
        {
            System.out.print('\b');
        }
        System.out.println("ok");

        System.out.print("testing server and router shutdown... ");
        System.out.flush();
        backend.shutdown();
        communicator().setDefaultRouter(null);
        Ice.ObjectPrx adminBase = communicator().stringToProxy("Glacier2/admin:tcp -h 127.0.0.1 -p 12348 -t 10000");
        Glacier2.AdminPrx admin = Glacier2.AdminPrxHelper.checkedCast(adminBase);
        test(admin != null);
        admin.shutdown();
        try
        {
           admin.ice_ping();
           test(false);
        }
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }

        return 0;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }
}
