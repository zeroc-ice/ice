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
        if (!b)
        {
            throw new RuntimeException();
        }
    }

    private static boolean
    find(Object[] array, Object elem)
    {
	for(int i = 0; i < array.length; i++)
	{
	    if(array[i].equals(elem))
	    {
		return true;
	    }
	}
	return false;
    }
    
    private static boolean
    findIdentity(Ice.ObjectPrx[] array, String elem)
    {
	for(int i = 0; i < array.length; i++)
	{
	    if(array[i].ice_getIdentity().equals(Ice.Util.stringToIdentity(elem)))
	    {
		return true;
	    }
	}
	return false;
    }
    
    public static void
    allCommonTests(Ice.Communicator communicator)
    {
	IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
	test(admin != null);
	
	System.out.print("test server registration... ");
	System.out.flush();
	String[] serverNames = admin.getAllServerNames();
	test(find(serverNames, "Server1"));
	test(find(serverNames, "Server2"));
	test(find(serverNames, "IceBox1"));
	test(find(serverNames, "IceBox2"));
	System.out.println("ok");

	System.out.print("testing adapter registration... ");
	System.out.flush();
	String[] adapterIds = admin.getAllAdapterIds();
	test(find(adapterIds, "Server1.Server"));
	test(find(adapterIds, "Server2.Server"));
	test(find(adapterIds, "IceBox1.Service1.Service1"));
	test(find(adapterIds, "IceBox1Service2Adapter"));
	test(find(adapterIds, "IceBox2.Service1.Service1"));
	test(find(adapterIds, "IceBox2Service2Adapter"));
	System.out.println("ok");

	IcePack.QueryPrx query = IcePack.QueryPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Query"));
	test(query != null);

	System.out.print("testing object registration... ");
	System.out.flush();
	try
	{
	    Ice.ObjectPrx[] objects = query.findAllObjectsWithType("::Test");
	    test(findIdentity(objects,"Server1"));
	    test(findIdentity(objects,"Server2"));
	    test(findIdentity(objects,"IceBox1-Service1"));
	    test(findIdentity(objects,"IceBox1-Service2"));
	    test(findIdentity(objects,"IceBox2-Service1"));
	    test(findIdentity(objects,"IceBox2-Service2"));
	}
	catch(IcePack.ObjectNotExistException ex)
	{
	    test(false);
	}

	try
	{
	    Ice.ObjectPrx obj = query.findObjectByType("::Test");
	    String id = Ice.Util.identityToString(obj.ice_getIdentity());
	    test(id.equals("Server1") || id.equals("Server2") || 
		 id.equals("IceBox1-Service1") || id.equals("IceBox1-Service2") ||
		 id.equals("IceBox2-Service1") || id.equals("IceBox2-Service2"));
	}
	catch(IcePack.ObjectNotExistException ex)
	{
	    test(false);
	}

	try
	{
	    Ice.ObjectPrx obj = query.findObjectByType("::Foo");
	    test(false);
	}
	catch(IcePack.ObjectNotExistException ex)
	{
	}

	System.out.println("ok");
    }

    public static void
    allTests(Ice.Communicator communicator)
    {
	allCommonTests(communicator);

	//
	// Ensure that all server and service objects are reachable.
	//
	// The identity for the test object in deployed server or services
	// is the name of the service or server. The object adapter name
	// is Adapter prefixed with the name of the service or
	// server. Ensure we can reach each object.
	//
	System.out.print("pinging server objects... ");
	System.out.flush();

	TestIntfPrx obj;

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server1@Server1.Server"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server2@Server2.Server"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox1-Service2@IceBox1Service2Adapter"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox2-Service1@IceBox2.Service1.Service1"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
    
	System.out.println("ok");

	System.out.print("testing server configuration... ");
	System.out.flush();

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server1@Server1.Server"));
	test(obj.getProperty("Type").equals("Server"));
	test(obj.getProperty("Name").equals("Server1"));

	test(obj.getProperty("Variable").equals("val0prop"));
	test(obj.getProperty("Variable1").equals(""));
	test(obj.getProperty("Variable2").equals(""));

	System.out.println("ok");

	System.out.print("testing service configuration... ");
	System.out.flush();

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
	test(obj.getProperty("Service1.Type").equals("standard"));
	test(obj.getProperty("Service1.ServiceName").equals("Service1"));
    
	test(obj.getProperty("Service1.InheritedVariable").equals("inherited"));

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox2-Service2@IceBox2Service2Adapter"));
	test(obj.getProperty("Service2.Type").equals("freeze"));
	test(obj.getProperty("Service2.ServiceName").equals("Service2"));

	test(obj.getProperty("Service2.DebugProperty").equals(""));
	test(obj.getProperty("Service1.DebugProperty").equals(""));
    
	System.out.println("ok");

	System.out.print("testing server options... ");
	System.out.flush();

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server1@Server1.Server"));
	test(obj.getProperty("Test.Test").equals("2"));
	test(obj.getProperty("Test.Test1").equals("0"));

	IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
	test(admin != null);

	System.out.println("ok");
    }

    public static void
    allTestsWithTarget(Ice.Communicator communicator)
    {
	allCommonTests(communicator);

	IcePack.AdminPrx admin = IcePack.AdminPrxHelper.checkedCast(communicator.stringToProxy("IcePack/Admin"));
	test(admin != null);

	System.out.print("pinging server objects... ");
	System.out.flush();

	//
	// Application is deployed with Server1.manual which disables on demand activation.
	//
	try
	{
	    admin.setServerActivation("Server1", IcePack.ServerActivation.Manual);
	}
	catch(IcePack.ServerNotExistException ex)
	{
	    test(false);
	}
	catch(IcePack.NodeUnreachableException ex)
	{
	    test(false);
	}

	TestIntfPrx obj;
	try
	{
	    obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server1@Server1.Server"));
	    test(false);
	}
	catch(Ice.LocalException ex)
	{
	}

	try
	{
	    admin.startServer("Server1");
	}
	catch(IcePack.ServerNotExistException ex)
	{
	    test(false);
	}
	catch(IcePack.NodeUnreachableException ex)
	{
	    test(false);
	}

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server1@Server1.Server"));
	test(obj.getProperty("Mode").equals("manual"));
	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("Server2@Server2.Server"));

	System.out.println("ok");

	System.out.print("testing service configuration... ");

	obj = TestIntfPrxHelper.checkedCast(communicator.stringToProxy("IceBox1-Service1@IceBox1.Service1.Service1"));
	test(obj.getProperty("Service1.DebugProperty").equals("debug"));

	System.out.println("ok");
    }
}
