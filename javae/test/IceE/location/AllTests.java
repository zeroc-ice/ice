// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator)
    {
	ServerManagerPrx manager = ServerManagerPrxHelper.checkedCast(
	    communicator.stringToProxy("ServerManager :default -t 10000 -p 12345"));
	test(manager != null);

	System.out.print("testing stringToProxy... ");
        System.out.flush();
	Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base2 = communicator.stringToProxy("test @ TestAdapter");
	Ice.ObjectPrx base3 = communicator.stringToProxy("test");
	Ice.ObjectPrx base4 = communicator.stringToProxy("ServerManager");
	System.out.println("ok");

	//
	// Start a server, get the port of the adapter it's listening on,
	// and add it to the configuration so that the client can locate
	// the TestAdapter adapter.
	//
	System.out.print("starting server... ");
        System.out.flush();
	manager.startServer();
	System.out.println("ok");

	System.out.print("testing checked cast... ");
        System.out.flush();
	TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
	test(obj != null);
	TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
	test(obj2 != null);
	TestIntfPrx obj3 = TestIntfPrxHelper.checkedCast(base3);
	test(obj3 != null);
	ServerManagerPrx obj4 = ServerManagerPrxHelper.checkedCast(base4);
	test(obj4 != null);
	System.out.println("ok");
 
	System.out.print("testing object reference from server... ");
        System.out.flush();
	HelloPrx hello = obj.getHello();
	hello.sayHello();
	System.out.println("ok");

	System.out.print("shutdown server... ");
        System.out.flush();
	obj.shutdown();
	System.out.println("ok");

	System.out.print("restarting server... ");
        System.out.flush();
	manager.startServer();
	System.out.println("ok");

	System.out.print("testing whether server is still reachable... ");
        System.out.flush();
	try
	{
	    obj2.ice_ping();
	}
	catch(Ice.LocalException ex)
	{
	    test(false);
	}
	System.out.println("ok");    
    
	System.out.print("testing object reference from server... ");
        System.out.flush();
	hello.sayHello();
	System.out.println("ok");

	System.out.print("testing reference with unknown identity... ");
	System.out.flush();
	try
	{
	    base = communicator.stringToProxy("unknown/unknown");
	    base.ice_ping();
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    test(ex.kindOfObject.equals("object"));
	    test(ex.id.equals("unknown/unknown"));
	}
	System.out.println("ok");	

	System.out.print("testing reference with unknown adapter... ");
	System.out.flush();
	try
	{
	    base = communicator.stringToProxy("test @ TestAdapterUnknown");
	    base.ice_ping();
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    test(ex.kindOfObject.equals("object adapter"));
	    test(ex.id.equals("TestAdapterUnknown"));
	}
	System.out.println("ok");	

	System.out.print("shutdown server... ");
        System.out.flush();
	obj.shutdown();
	System.out.println("ok");

	System.out.print("testing whether server is gone... ");
        System.out.flush();
	try
	{
	    obj2.ice_ping();
	    test(false);
	}
        catch(Ice.LocalException ex)
        {
            System.out.println("ok");
        }

	System.out.print("shutdown server manager... ");
        System.out.flush();
	manager.shutdown();
	System.out.println("ok");
    }
}
