// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.SessionHelper
{
    public class Client : TestHelper
    {
        private readonly object _mutex = new object();

        public class SessionCallback1 : ISessionCallback
        {
            private readonly Client _app;

            public SessionCallback1(Client app) => _app = app;

            public void Connected(Glacier2.SessionHelper session) => Assert(false);

            public void Disconnected(Glacier2.SessionHelper session) => Assert(false);

            public void ConnectFailed(Glacier2.SessionHelper session, Exception exception)
            {
                try
                {
                    throw exception;
                }
                catch (PermissionDeniedException)
                {
                    Console.Out.WriteLine("ok");
                }
                catch
                {
                    Assert(false);
                }
                finally
                {
                    _app.WakeUp();
                }
            }

            public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator != null);
        }

        public class SessionCallback2 : ISessionCallback
        {
            private readonly Client _app;

            public SessionCallback2(Client app) => _app = app;

            public void Connected(Glacier2.SessionHelper session)
            {
                Console.Out.WriteLine("ok");
                _app.WakeUp();
            }

            public void Disconnected(Glacier2.SessionHelper session)
            {
                Console.Out.WriteLine("ok");
                _app.WakeUp();
            }

            public void ConnectFailed(Glacier2.SessionHelper session, Exception ex)
            {
                Console.Out.WriteLine(ex.ToString());
                Assert(false);
            }

            public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator != null);
        }

        public class SessionCallback3 : ISessionCallback
        {
            private readonly Client _app;

            public SessionCallback3(Client app) => _app = app;

            public void Connected(Glacier2.SessionHelper session) => Assert(false);

            public void Disconnected(Glacier2.SessionHelper session) => Assert(false);

            public void ConnectFailed(Glacier2.SessionHelper session, Exception exception)
            {
                try
                {
                    throw exception;
                }
                catch (ConnectionRefusedException)
                {
                    Console.Out.WriteLine("ok");
                }
                catch
                {
                    Assert(false);
                }
                finally
                {
                    _app.WakeUp();
                }
            }

            public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator != null);
        }

        public class SessionCallback4 : ISessionCallback
        {
            private readonly Client _app;

            public SessionCallback4(Client app) => _app = app;

            public void Connected(Glacier2.SessionHelper session) => Assert(false);

            public void Disconnected(Glacier2.SessionHelper session) => Assert(false);

            public void ConnectFailed(Glacier2.SessionHelper session, Exception exception)
            {
                try
                {
                    Assert(exception is CommunicatorDisposedException ||
                           exception is FormatException ||
                           exception is OperationCanceledException);
                    Console.Out.WriteLine("ok");
                }
                finally
                {
                    _app.WakeUp();
                }
            }

            public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator != null);
        }

        public override Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Test.Protocol"] = "ice1";
            properties["Ice.Default.Router"] = GetTestProxy("Glacier2/router", properties, 50);

            using Communicator communicator = Initialize(properties);

            var factory = new SessionFactoryHelper(new SessionCallback1(this), properties);
            Glacier2.SessionHelper? session = null;

            // Test to create a session with wrong userid/password
            lock (_mutex)
            {
                Console.Out.Write("testing SessionHelper connect with wrong userid/password... ");
                Console.Out.Flush();

                factory.Transport = Transport;
                session = factory.Connect("userid", "xxx");
                while (true)
                {
                    try
                    {
                        if (!Monitor.Wait(_mutex, 30000))
                        {
                            Assert(false);
                        }
                        break;
                    }
                    catch (ThreadInterruptedException)
                    {
                    }
                }
                Assert(!session.IsConnected);
            }

            properties.Remove("Ice.Default.Router");
            factory = new SessionFactoryHelper(new SessionCallback4(this), properties);
            lock (_mutex)
            {
                Console.Out.Write("testing SessionHelper connect interrupt... ");
                Console.Out.Flush();
                factory.RouterHost = Host;
                factory.Port = BasePort + 1;
                factory.Transport = Transport;
                session = factory.Connect("userid", "abc123");

                Thread.Sleep(100);
                session.Destroy();

                while (true)
                {
                    try
                    {
                        if (!Monitor.Wait(_mutex, 30000))
                        {
                            Assert(false);
                        }
                        break;
                    }
                    catch (ThreadInterruptedException)
                    {
                    }
                }
                Assert(!session.IsConnected);
            }

            factory = new SessionFactoryHelper(new SessionCallback2(this), properties);
            lock (_mutex)
            {
                Console.Out.Write("testing SessionHelper connect... ");
                Console.Out.Flush();
                factory.RouterHost = Host;
                factory.Port = BasePort + 50;
                factory.Transport = Transport;
                session = factory.Connect("userid", "abc123");
                while (true)
                {
                    try
                    {
                        if (!Monitor.Wait(_mutex, 30000))
                        {
                            Assert(false);
                        }
                        break;
                    }
                    catch (ThreadInterruptedException)
                    {
                    }
                }

                Console.Out.Write("testing SessionHelper isConnected after connect... ");
                Console.Out.Flush();
                Assert(session.IsConnected);
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper categoryForClient after connect... ");
                Console.Out.Flush();
                try
                {
                    Assert(session.GetCategoryForClient().Length > 0);
                }
                catch (SessionNotExistException)
                {
                    Assert(false);
                }
                Console.Out.WriteLine("ok");

                Assert(session.Session == null);

                Console.Out.Write("testing stringToProxy for server object... ");
                Console.Out.Flush();
                var twoway = ICallbackPrx.Parse(GetTestProxy("callback", 0), session.Communicator!);
                Console.Out.WriteLine("ok");

                Console.Out.Write("pinging server after session creation... ");
                Console.Out.Flush();
                twoway.IcePing();
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing server shutdown... ");
                Console.Out.Flush();
                twoway.Shutdown();
                Console.Out.WriteLine("ok");

                Assert(session.Communicator != null);
                Console.Out.Write("testing SessionHelper destroy... ");
                Console.Out.Flush();
                session.Destroy();
                while (true)
                {
                    try
                    {
                        if (!Monitor.Wait(_mutex, 30000))
                        {
                            Assert(false);
                        }
                        break;
                    }
                    catch (ThreadInterruptedException)
                    {
                    }
                }

                Console.Out.Write("testing SessionHelper isConnected after destroy... ");
                Console.Out.Flush();
                Assert(session.IsConnected == false);
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper categoryForClient after destroy... ");
                Console.Out.Flush();
                try
                {
                    Assert(session.GetCategoryForClient().Length > 0);
                    Assert(false);
                }
                catch (SessionNotExistException)
                {
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper session after destroy... ");
                Assert(session.Session == null);
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper communicator after destroy... ");
                Console.Out.Flush();
                try
                {
                    IObjectPrx.Parse("dummy", session.Communicator!).IcePing();
                    Assert(false);
                }
                catch (CommunicatorDisposedException)
                {
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("uninstalling router with communicator... ");
                Console.Out.Flush();
                communicator.DefaultRouter = null;
                Console.Out.WriteLine("ok");

                IProcessPrx process;
                {
                    Console.Out.Write("testing stringToProxy for process object... ");
                    process = IProcessPrx.Parse(GetTestProxy("Glacier2/admin -f Process", 51), communicator);
                    Console.Out.WriteLine("ok");
                }

                Console.Out.Write("testing Glacier2 shutdown... ");
                process.Shutdown();
                try
                {
                    process.IcePing();
                    Assert(false);
                }
                catch
                {
                    Console.Out.WriteLine("ok");
                }
            }

            factory = new SessionFactoryHelper(new SessionCallback3(this), properties);
            lock (_mutex)
            {
                Console.Out.Write("testing SessionHelper connect after router shutdown... ");
                Console.Out.Flush();

                factory.RouterHost = Host;
                factory.Port = BasePort + 50;
                factory.Transport = Transport;
                session = factory.Connect("userid", "abc123");
                while (true)
                {
                    try
                    {
                        if (!Monitor.Wait(_mutex, 30000))
                        {
                            Assert(false);
                        }
                        break;
                    }
                    catch (ThreadInterruptedException)
                    {
                    }
                }

                Console.Out.Write("testing SessionHelper isConnect after connect failure... ");
                Console.Out.Flush();
                Assert(session.IsConnected == false);
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper communicator after connect failure... ");
                Console.Out.Flush();
                try
                {
                    IObjectPrx.Parse("dummy", session.Communicator!).IcePing();
                    Assert(false);
                }
                catch (CommunicatorDisposedException)
                {
                }
                Console.Out.WriteLine("ok");

                Console.Out.Write("testing SessionHelper destroy after connect failure... ");
                Console.Out.Flush();
                session.Destroy();
                Console.Out.WriteLine("ok");
            }
            return Task.CompletedTask;
        }

        public void WakeUp()
        {
            lock (_mutex)
            {
                Monitor.Pulse(_mutex);
            }
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
