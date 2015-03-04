// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceGrid.simple;
import java.io.PrintWriter;

import test.IceGrid.simple.Test.TestIntfPrx;
import test.IceGrid.simple.Test.TestIntfPrxHelper;

public class AllTests
{
    static private class SessionKeepAliveThread extends Thread
    {
        SessionKeepAliveThread(IceGrid.AdminSessionPrx session, long timeout)
        {
            _session = session;
            _timeout = timeout;
            _terminated = false;
        }

        synchronized public void
        run()
        {
            while(!_terminated)
            {
                try
                {
                    wait(_timeout);
                }
                catch(InterruptedException e)
                {
                }
                if(_terminated)
                {
                    break;
                }
                try
                {
                    _session.keepAlive();
                }
                catch(Ice.LocalException ex)
                {
                    break;
                }
            }
        }

        synchronized private void
        terminate()
        {
            _terminated = true;
            notify();
        }

        final private IceGrid.AdminSessionPrx _session;
        final private long _timeout;
        private boolean _terminated;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public static void
    allTests(Ice.Communicator communicator, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        String ref = "test @ TestAdapter";
        Ice.ObjectPrx base = communicator.stringToProxy(ref);
        test(base != null);
        out.println("ok");

        out.print("testing IceGrid.Locator is present... ");
        IceGrid.LocatorPrx locator = IceGrid.LocatorPrxHelper.uncheckedCast(base);
        test(locator != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        out.println("ok");
        
        out.print("pinging server... ");
        out.flush();
        obj.ice_ping();
        out.println("ok");
        
        out.print("shutting down server... ");
        out.flush();
        obj.shutdown();
        out.println("ok");       
    }

    public static void
    allTestsWithDeploy(Ice.Communicator communicator, PrintWriter out)
    {
        out.print("testing stringToProxy... ");
        out.flush();
        Ice.ObjectPrx base = communicator.stringToProxy("test @ TestAdapter");
        test(base != null);
        Ice.ObjectPrx base2 = communicator.stringToProxy("test");
        test(base2 != null);
        out.println("ok");

        out.print("testing checked cast... ");
        out.flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(base);
        test(obj != null);
        test(obj.equals(base));
        TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(base2);
        test(obj2 != null);
        test(obj2.equals(base2));
        out.println("ok");
        
        out.print("pinging server... ");
        out.flush();
        obj.ice_ping();
        obj2.ice_ping();
        out.println("ok");

        out.print("testing encoding versioning... ");
        out.flush();
        Ice.ObjectPrx base10 = communicator.stringToProxy("test10 @ TestAdapter10");
        test(base10 != null);
        Ice.ObjectPrx base102 = communicator.stringToProxy("test10");
        test(base102 != null);
        try
        {
            base10.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        try
        {
            base102.ice_ping();
            test(false);
        }
        catch(Ice.NoEndpointException ex)
        {
        }
        base10 = base10.ice_encodingVersion(Ice.Util.Encoding_1_0);
        base102 = base102.ice_encodingVersion(Ice.Util.Encoding_1_0);
        base10.ice_ping();
        base102.ice_ping();
        out.println("ok");

        out.print("testing reference with unknown identity... ");
        out.flush();
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
        out.println("ok");       

        out.print("testing reference with unknown adapter... ");
        out.flush();
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
        out.println("ok");       

        IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
            communicator.stringToProxy("IceGrid/Registry"));
        test(registry != null);
        IceGrid.AdminSessionPrx session = null;
        try
        {
            session = registry.createAdminSession("foo", "bar");
        }
        catch(IceGrid.PermissionDeniedException e)
        {
            test(false);
        }

        SessionKeepAliveThread keepAlive = new SessionKeepAliveThread(session, registry.getSessionTimeout()/2);
        keepAlive.start();

        IceGrid.AdminPrx admin = session.getAdmin();
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
        catch(IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(IceGrid.DeploymentException ex)
        {
            test(false);
        }

        out.print("testing whether server is still reachable... ");
        out.flush();
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
        catch(IceGrid.DeploymentException ex)
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
        out.println("ok");

        try
        {
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException ex)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException ex)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException ex)
        {
            test(false);
        }
        catch(IceGrid.DeploymentException ex)
        {
            test(false);
        }

        keepAlive.terminate();
        try
        {
            keepAlive.join();
        }
        catch(InterruptedException e)
        {
        }
        session.destroy();
    }
}
