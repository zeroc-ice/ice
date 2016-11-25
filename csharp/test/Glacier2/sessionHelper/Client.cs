// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;
using System;
using System.Reflection;
using System.Threading;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : TestCommon.Application
{
    public class SessionCallback1 : Glacier2.SessionCallback
    {
        public
        SessionCallback1(Client app)
        {
            _app = app;
        }

        public void
        connected(Glacier2.SessionHelper session)
        {
             test(false);
        }

        public void
        disconnected(Glacier2.SessionHelper session)
        {
            test(false);
        }

        public void
        connectFailed(Glacier2.SessionHelper session, Exception exception)
        {
            try
            {
                throw exception;
            }
            catch(Glacier2.PermissionDeniedException)
            {
                Console.Out.WriteLine("ok");
                _app.wakeUp();
            }
            catch(Exception)
            {
                test(false);
            }
        }

        public void
        createdCommunicator(Glacier2.SessionHelper session)
        {
            test(session.communicator() != null);
        }

        private Client _app;
    }

    public class SessionCallback2 : Glacier2.SessionCallback
    {
        public
        SessionCallback2(Client app)
        {
            _app = app;
        }

        public void
        connected(Glacier2.SessionHelper session)
        {
            Console.Out.WriteLine("ok");
            _app.wakeUp();
        }

        public void
        disconnected(Glacier2.SessionHelper session)
        {
            Console.Out.WriteLine("ok");
            _app.wakeUp();
        }

        public void
        connectFailed(Glacier2.SessionHelper session, Exception ex)
        {
            Console.Out.WriteLine(ex.ToString());
            test(false);
        }

        public void
        createdCommunicator(Glacier2.SessionHelper session)
        {
            test(session.communicator() != null);
        }

        private Client _app;
    }

    public class SessionCallback3 : Glacier2.SessionCallback
    {
        public
        SessionCallback3(Client app)
        {
            _app = app;
        }

        public void
        connected(Glacier2.SessionHelper session)
        {
             test(false);
        }

        public void
        disconnected(Glacier2.SessionHelper session)
        {
            test(false);
        }

        public void
        connectFailed(Glacier2.SessionHelper session, Exception exception)
        {
            try
            {
                throw exception;
            }
            catch(Ice.ConnectionRefusedException)
            {
                Console.Out.WriteLine("ok");
                _app.wakeUp();
            }
            catch(Exception)
            {
                test(false);
            }
        }

        public void
        createdCommunicator(Glacier2.SessionHelper session)
        {
            test(session.communicator() != null);
        }

        private Client _app;
    }

    public class SessionCallback4 : Glacier2.SessionCallback
    {
        public
        SessionCallback4(Client app)
        {
            _app = app;
        }

        public void
        connected(Glacier2.SessionHelper session)
        {
             test(false);
        }

        public void
        disconnected(Glacier2.SessionHelper session)
        {
            test(false);
        }

        public void
        connectFailed(Glacier2.SessionHelper session, Exception exception)
        {
            try
            {
                throw exception;
            }
            catch(Ice.CommunicatorDestroyedException)
            {
                Console.Out.WriteLine("ok");
                _app.wakeUp();
            }
            catch(Exception)
            {
                test(false);
            }
        }

        public void
        createdCommunicator(Glacier2.SessionHelper session)
        {
            test(session.communicator() != null);
        }

        private Client _app;
    }

    public override int run(string[] args)
    {
        string protocol = getTestProtocol();
        string host = getTestHost();

        _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCallback1(this));

        //
        // Test to create a session with wrong userid/password
        //
        lock(this)
        {
            Console.Out.Write("testing SessionHelper connect with wrong userid/password... ");
            Console.Out.Flush();

            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "xxx");
            while(true)
            {
                try
                {
                    Monitor.Wait(this);
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
            }
        }

        _initData.properties.setProperty("Ice.Default.Router", "");
        _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCallback4(this));
        lock(this)
        {
            Console.Out.Write("testing SessionHelper connect interrupt... ");
            Console.Out.Flush();
            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(1));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");

            Thread.Sleep(100);
            _session.destroy();

            while(true)
            {
                try
                {
                    Monitor.Wait(this);
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
            }
        }

        _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCallback2(this));
        lock(this)
        {
            Console.Out.Write("testing SessionHelper connect... ");
            Console.Out.Flush();
            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(10));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");
            while(true)
            {
                try
                {
                    Monitor.Wait(this);
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
            }

            Console.Out.Write("testing SessionHelper isConnected after connect... ");
            Console.Out.Flush();
            test(_session.isConnected());
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper categoryForClient after connect... ");
            Console.Out.Flush();
            try
            {
                test(!_session.categoryForClient().Equals(""));
            }
            catch(Glacier2.SessionNotExistException)
            {
                test(false);
            }
            Console.Out.WriteLine("ok");

//             try
//             {
//                 test(_session.session() != null);
//             }
//             catch(Glacier2.SessionNotExistException ex)
//             {
//                 test(false);
//             }

            Console.Out.Write("testing stringToProxy for server object... ");
            Console.Out.Flush();
            Ice.ObjectPrx @base = _session.communicator().stringToProxy("callback:" + getTestEndpoint(0));
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server after session creation... ");
            Console.Out.Flush();
            @base.ice_ping();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing checked cast for server object... ");
            Console.Out.Flush();
            CallbackPrx twoway = CallbackPrxHelper.checkedCast(@base);
            test(twoway != null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            twoway.shutdown();
            Console.Out.WriteLine("ok");

            test(_session.communicator() != null);
            Console.Out.Write("testing SessionHelper destroy... ");
            Console.Out.Flush();
            _session.destroy();
            while(true)
            {
                try
                {
                    Monitor.Wait(this);
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
            }

            Console.Out.Write("testing SessionHelper isConnected after destroy... ");
            Console.Out.Flush();
            test(_session.isConnected() == false);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper categoryForClient after destroy... ");
            Console.Out.Flush();
            try
            {
                test(!_session.categoryForClient().Equals(""));
                test(false);
            }
            catch(Glacier2.SessionNotExistException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper session after destroy... ");
            try
            {
                _session.session();
                test(false);
            }
            catch(Glacier2.SessionNotExistException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper communicator after destroy... ");
            Console.Out.Flush();
            try
            {
                test(_session.communicator() != null);
                _session.communicator().stringToProxy("dummy");
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException)
            {
            }
            Console.Out.WriteLine("ok");


            Console.Out.Write("uninstalling router with communicator... ");
            Console.Out.Flush();
            communicator().setDefaultRouter(null);
            Console.Out.WriteLine("ok");

            Ice.ObjectPrx processBase;
            {
                Console.Out.Write("testing stringToProxy for process object... ");
                processBase = communicator().stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(11, "tcp"));
                Console.Out.WriteLine("ok");
            }


            Ice.ProcessPrx process;
            {
                Console.Out.Write("testing checked cast for admin object... ");
                process = Ice.ProcessPrxHelper.checkedCast(processBase);
                test(process != null);
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing Glacier2 shutdown... ");
            process.shutdown();
            try
            {
                process.ice_ping();
                test(false);
            }
            catch(Ice.LocalException)
            {
                Console.Out.WriteLine("ok");
            }
        }

        _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCallback3(this));
        lock(this)
        {
            Console.Out.Write("testing SessionHelper connect after router shutdown... ");
            Console.Out.Flush();

            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(10));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");
            while(true)
            {
                try
                {
                    Monitor.Wait(this);
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
            }

            Console.Out.Write("testing SessionHelper isConnect after connect failure... ");
            Console.Out.Flush();
            test(_session.isConnected() == false);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper communicator after connect failure... ");
            Console.Out.Flush();
            try
            {
                test(_session.communicator() != null);
                _session.communicator().stringToProxy("dummy");
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper destroy after connect failure... ");
            Console.Out.Flush();
            _session.destroy();
            Console.Out.WriteLine("ok");
        }

        return 0;
    }

    public void
    wakeUp()
    {
        lock(this)
        {
            Monitor.Pulse(this);
        }
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        _initData = base.getInitData(ref args);
        _initData.properties.setProperty("Ice.Warn.Connections", "0");
        _initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" +
                                         getTestEndpoint(_initData.properties, 10));
        _initData.dispatcher = delegate(Action action, Ice.Connection connection)
        {
            action();
        };
        return _initData;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }

    private Glacier2.SessionHelper _session;
    private Glacier2.SessionFactoryHelper _factory;
    private Ice.InitializationData _initData;
}
