// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Glacier2;
using Test;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Reflection;
using System.Threading;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        public App(string[] args)
        {
            me = this;

            _initData = new Ice.InitializationData();
            _initData.properties = Ice.Util.createProperties(ref args);
            _initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:default -p 12347");
#if COMPACT
            _initData.dispatcher = delegate(Ice.VoidAction action, Ice.Connection connection)
#else
            _initData.dispatcher = delegate(System.Action action, Ice.Connection connection)
#endif
                {
                    action();
                };
        }

        public class SessionCalback1 : Glacier2.SessionCallback
        {
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
            connectFailed(Glacier2.SessionHelper session, System.Exception exception)
            {
                try
                {
                    throw exception;
                }
                catch(Glacier2.PermissionDeniedException)
                {
                    Console.Out.WriteLine("ok");
                    lock(me)
                    {
                        wakeUp();
                    }
                }
                catch(System.Exception)
                {
                    test(false);
                }
            }

            public void
            createdCommunicator(Glacier2.SessionHelper session)
            {
                test(session.communicator() != null);
            }
        }

        public class SessionCalback2 : Glacier2.SessionCallback
        {
            public void
            connected(Glacier2.SessionHelper session)
            {
                Console.Out.WriteLine("ok");
                lock(me)
                {
                    wakeUp();
                }
            }

            public void
            disconnected(Glacier2.SessionHelper session)
            {
                Console.Out.WriteLine("ok");
                lock(me)
                {
                    wakeUp();
                }
            }

            public void
            connectFailed(Glacier2.SessionHelper session, System.Exception ex)
            {
                Console.Out.WriteLine(ex.ToString());
                test(false);
            }

            public void
            createdCommunicator(Glacier2.SessionHelper session)
            {
                test(session.communicator() != null);
            }
        }

        public class SessionCalback3 : Glacier2.SessionCallback
        {
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
            connectFailed(Glacier2.SessionHelper session, System.Exception exception)
            {
                try
                {
                    throw exception;
                }
                catch(Ice.ConnectionRefusedException)
                {
                    Console.Out.WriteLine("ok");
                    lock(me)
                    {
                        wakeUp();
                    }
                }
                catch(System.Exception)
                {
                    test(false);
                }
            }

            public void
            createdCommunicator(Glacier2.SessionHelper session)
            {
                test(session.communicator() != null);
            }
        }

        public class SessionCalback4 : Glacier2.SessionCallback
        {
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
            connectFailed(Glacier2.SessionHelper session, System.Exception exception)
            {
                try
                {
                    throw exception;
                }
                catch(Ice.CommunicatorDestroyedException)
                {
                    Console.Out.WriteLine("ok");
                    lock(me)
                    {
                        wakeUp();
                    }
                }
                catch(System.Exception)
                {
                    test(false);
                }
            }

            public void
            createdCommunicator(Glacier2.SessionHelper session)
            {
                test(session.communicator() != null);
            }
        }

        public Ice.InitializationData getInitData()
        {
            return _initData;
        }

        public override int run(string[] args)
        {
            String protocol = communicator().getProperties().getPropertyWithDefault("Ice.Default.Protocol", "tcp");
            String host = communicator().getProperties().getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
            _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCalback1());

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
#if COMPACT
                    System.Threading.Monitor.Wait(this, 30000);
                    break;
#else
                    try
                    {
                        System.Threading.Monitor.Wait(this, 30000);
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
#endif
                }
                test(!_session.isConnected());
            }

            _initData.properties.setProperty("Ice.Default.Router", "");
            _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCalback4());
            lock(this)
            {
                Console.Out.Write("testing SessionHelper connect interrupt... ");
                Console.Out.Flush();
                _factory.setRouterHost(host);
                _factory.setPort(12011);
                _factory.setProtocol(protocol);
                _session = _factory.connect("userid", "abc123");

                System.Threading.Thread.Sleep(100);
                _session.destroy();

                while(true)
                {
#if COMPACT
                    System.Threading.Monitor.Wait(this, 30000);
                    break;
#else
                    try
                    {
                        System.Threading.Monitor.Wait(this, 30000);
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
#endif
                }
                test(!_session.isConnected());
            }

            _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCalback2());
            lock(this)
            {
                Console.Out.Write("testing SessionHelper connect... ");
                Console.Out.Flush();
                _factory.setRouterHost(host);
                _factory.setPort(12347);
                _factory.setProtocol(protocol);
                _session = _factory.connect("userid", "abc123");
                while(true)
                {
#if COMPACT
                    System.Threading.Monitor.Wait(this, 30000);
                    break;
#else
                    try
                    {
                        System.Threading.Monitor.Wait(this, 30000);
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
#endif
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
                Ice.ObjectPrx @base = _session.communicator().stringToProxy("callback:default -p 12010");
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
#if COMPACT
                    System.Threading.Monitor.Wait(this);
                    break;
#else
                    try
                    {
                        System.Threading.Monitor.Wait(this);
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
#endif
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
                    processBase = communicator().stringToProxy("Glacier2/admin -f Process:default -h \"" + host + "\" -p 12348");
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

            _factory = new Glacier2.SessionFactoryHelper(_initData, new SessionCalback3());
            lock(this)
            {
                Console.Out.Write("testing SessionHelper connect after router shutdown... ");
                Console.Out.Flush();

                _factory.setRouterHost(host);
                _factory.setPort(12347);
                _factory.setProtocol(protocol);
                _session = _factory.connect("userid", "abc123");
                while(true)
                {
#if COMPACT
                    System.Threading.Monitor.Wait(this);
                    break;
#else
                    try
                    {
                        System.Threading.Monitor.Wait(this);
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
#endif
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

        public static void
        wakeUp()
        {
            System.Threading.Monitor.Pulse(me);
        }

        private static void
        test(bool b)
        {
            if(!b)
            {
                throw new Exception();
            }
        }

        public static App me;
        private Ice.InitializationData _initData;
        private Glacier2.SessionHelper _session;
        private Glacier2.SessionFactoryHelper _factory;
    }

    public static int Main(string[] args)
    {
        App app = new App(args);
        app.getInitData().properties.setProperty("Ice.Warn.Connections", "0");
        return app.main(args, app.getInitData());
    }
}
