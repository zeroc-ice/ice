// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using Test;

public class AllTests
{
    class SessionKeepAliveThread 
    {
        public SessionKeepAliveThread(IceGrid.AdminSessionPrx session, int timeout)
        {
            _session = session;
            _timeout = timeout;
            _terminated = false;
        }

        public void run()
        {
            lock(this)
            {
                while(!_terminated)
                {
                    System.Threading.Monitor.Wait(this, _timeout);
                    if(_terminated)
                    {
                        break;
                    }
                    try
                    {
                        _session.keepAlive();
                    }
                    catch(Ice.LocalException)
                    {
                        break;
                    }
                }
            }
        }

        public void terminate()
        {
            lock(this)
            {
                _terminated = true;
                System.Threading.Monitor.Pulse(this);
            }
        }

        private IceGrid.AdminSessionPrx _session;
        private int _timeout;
        private bool _terminated;
    }

    private static void
    test(bool b)
    {
        if(!b)
        {
            throw new Exception();
        }
    }

    public static void allTests(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        String rf = "test @ TestAdapter";
        Ice.ObjectPrx @base = communicator.stringToProxy(rf);
        test(@base != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.ice_ping();
        Console.Out.WriteLine("ok");
        
        System.Console.Out.Write("shutting down server... ");
        System.Console.Out.Flush();
        obj.shutdown();
        System.Console.Out.WriteLine("ok");
    }

    public static void
    allTestsWithDeploy(Ice.Communicator communicator)
    {
        Console.Out.Write("testing stringToProxy... ");
        Console.Out.Flush();
        Ice.ObjectPrx @base = communicator.stringToProxy("test @ TestAdapter");
        test(@base != null);
        Ice.ObjectPrx @base2 = communicator.stringToProxy("test");
        test(@base2 != null);
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing checked cast... ");
        Console.Out.Flush();
        TestIntfPrx obj = TestIntfPrxHelper.checkedCast(@base);
        test(obj != null);
        test(obj.Equals(@base));
        TestIntfPrx obj2 = TestIntfPrxHelper.checkedCast(@base2);
        test(obj2 != null);
        test(obj2.Equals(@base2));
        Console.Out.WriteLine("ok");
        
        Console.Out.Write("pinging server... ");
        Console.Out.Flush();
        obj.ice_ping();
        obj2.ice_ping();
        Console.Out.WriteLine("ok");

        Console.Out.Write("testing reference with unknown identity... ");
        Console.Out.Flush();
        try
        {
            communicator.stringToProxy("unknown/unknown").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object"));
            test(ex.id.Equals("unknown/unknown"));
        }
        Console.Out.WriteLine("ok");    

        Console.Out.Write("testing reference with unknown adapter... ");
        Console.Out.Flush();
        try
        {
            communicator.stringToProxy("test @ TestAdapterUnknown").ice_ping();
            test(false);
        }
        catch(Ice.NotRegisteredException ex)
        {
            test(ex.kindOfObject.Equals("object adapter"));
            test(ex.id.Equals("TestAdapterUnknown"));
        }
        Console.Out.WriteLine("ok");    

        IceGrid.RegistryPrx registry = IceGrid.RegistryPrxHelper.checkedCast(
            communicator.stringToProxy("IceGrid/Registry"));
        test(registry != null);
        IceGrid.AdminSessionPrx session = null;
        try
        {
            session = registry.createAdminSession("foo", "bar");
        }
        catch(IceGrid.PermissionDeniedException)
        {
            test(false);
        }

        SessionKeepAliveThread keepAlive = new SessionKeepAliveThread(session, registry.getSessionTimeout()/2);
        Thread keepAliveThread = new Thread(new ThreadStart(keepAlive.run));
        keepAliveThread.Start();

        IceGrid.AdminPrx admin = session.getAdmin();
        test(admin != null);

        try
        {
            admin.enableServer("server", false);
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        Console.Out.Write("testing whether server is still reachable... ");
        Console.Out.Flush();
        try
        {
            obj = TestIntfPrxHelper.checkedCast(@base);
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(@base2);
            test(false);
        }
        catch(Ice.NoEndpointException)
        {
        }
        
        try
        {
            admin.enableServer("server", true);
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        try
        {
            obj = TestIntfPrxHelper.checkedCast(@base);
        }
        catch(Ice.NoEndpointException)
        {
            test(false);
        }
        try
        {
            obj2 = TestIntfPrxHelper.checkedCast(@base2);
        }
        catch(Ice.NoEndpointException)
        {
            test(false);
        }
        Console.Out.WriteLine("ok");
        
        try
        {
            admin.stopServer("server");
        }
        catch(IceGrid.ServerNotExistException)
        {
            test(false);
        }
        catch(IceGrid.ServerStopException)
        {
            test(false);
        }
        catch(IceGrid.NodeUnreachableException)
        {
            test(false);
        }

        keepAlive.terminate();
        keepAliveThread.Join();
        session.destroy();
    }
}
