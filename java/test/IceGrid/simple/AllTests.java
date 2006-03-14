// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator)
    {
	System.out.print("testing stringToProxy... ");
	System.out.flush();
	String ref = "test @ TestAdapter";
	Ice.ObjectPrx base = communicator.stringToProxy(ref);
	test(base != null);
	System.out.println("ok");

	System.out.print("testing checked cast... ");
	System.out.flush();
	TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
	test(obj != null);
	test(obj.equals(base));
	System.out.println("ok");
	
	System.out.print("pinging server... ");
	System.out.flush();
	obj.ice_ping();
	System.out.println("ok");
	
	System.out.print("shutting down server... ");
	System.out.flush();
	obj.shutdown();
	System.out.println("ok");	
    }

    public static void
    allTestsWithDeploy(Ice.Communicator communicator)
    {
	System.out.print("testing stringToProxy... ");
	System.out.flush();
	Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
	test(base != null);
	Ice.ObjectPrx base2 = communicator.stringToProxy("test");
	test(base2 != null);
	System.out.println("ok");

	System.out.print("testing checked cast... ");
	System.out.flush();
	TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
	test(obj != null);
	test(obj.equals(base));
	TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
	test(obj2 != null);
	test(obj2.equals(base2));
	System.out.println("ok");
	
	System.out.print("pinging server... ");
	System.out.flush();
	obj.ice_ping();
	obj2.ice_ping();
	System.out.println("ok");

	System.out.print("testing reference with unknown identity... ");
	System.out.flush();
	try
	{
	    communicator.stringToProxy("unknown/unknown").ice_ping();
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
	    communicator.stringToProxy("test @ TestAdapterUnknown").ice_ping();
	    test(false);
	}
	catch(Ice.NotRegisteredException ex)
	{
	    test(ex.kindOfObject.equals("object adapter"));
	    test(ex.id.equals("TestAdapterUnknown"));
	}
	System.out.println("ok");	

 	IceGrid.AdminPrx admin = IceGrid.AdminPrxHelper.checkedCast(communicator.stringToProxy("IceGrid/Admin"));
	test(admin != null);

	try
	{
	    admin.enableServer("server", false);
	    admin.stopServer("server");
	}
	catch(IceGrid.ServerNotExistException ex)
	{
	    test(false);
	}
	catch(IceGrid.NodeUnreachableException ex)
	{
	    test(false);
	}

	System.out.print("testing whether server is still reachable... ");
	System.out.flush();
	try
	{
	    obj = TestIntfPrxHelper.checkedCast(base);
	    test(false);
	}
	catch(Ice.NoEndpointException ex)
	{
	}
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	    test(false);
	}
	catch(Ice.NoEndpointException ex)
	{
	}
	
	try
	{
	    admin.enableServer("server", true);
	}
	catch(IceGrid.ServerNotExistException ex)
	{
	    test(false);
	}
	catch(IceGrid.NodeUnreachableException ex)
	{
	    test(false);
	}

	try
	{
	    obj = TestIntfPrxHelper.checkedCast(base);
	}
	catch(Ice.NoEndpointException ex)
	{
	    test(false);
	}
	try
	{
	    obj2 = TestIntfPrxHelper.checkedCast(base2);
	}
	catch(Ice.NoEndpointException ex)
	{
	    test(false);
	}
	System.out.println("ok");

	try
	{
	    admin.stopServer("server");
	}
	catch(IceGrid.ServerNotExistException ex)
	{
	    test(false);
	}
	catch(IceGrid.NodeUnreachableException ex)
	{
	    test(false);
	}
    }
}
