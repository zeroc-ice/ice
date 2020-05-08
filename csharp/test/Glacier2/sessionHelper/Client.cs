//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Threading;
using Ice;

public class Client : TestHelper
{
    public class SessionCallback1 : Glacier2.ISessionCallback
    {
        public
        SessionCallback1(Client app)
        {
            _app = app;
        }

        public void
        Connected(Glacier2.SessionHelper session) => Assert(false);

        public void
        Disconnected(Glacier2.SessionHelper session) => Assert(false);

        public void
        ConnectFailed(Glacier2.SessionHelper session, Exception exception)
        {
            try
            {
                throw exception;
            }
            catch (Glacier2.PermissionDeniedException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (Exception)
            {
                Assert(false);
            }
            finally
            {
                _app.wakeUp();
            }
        }

        public void
        CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator() != null);

        private Client _app;
    }

    public class SessionCallback2 : Glacier2.ISessionCallback
    {
        public SessionCallback2(Client app) => _app = app;

        public void Connected(Glacier2.SessionHelper session)
        {
            Console.Out.WriteLine("ok");
            _app.wakeUp();
        }

        public void Disconnected(Glacier2.SessionHelper session)
        {
            Console.Out.WriteLine("ok");
            _app.wakeUp();
        }

        public void ConnectFailed(Glacier2.SessionHelper session, Exception ex)
        {
            Console.Out.WriteLine(ex.ToString());
            Assert(false);
        }

        public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator() != null);

        private readonly Client _app;
    }

    public class SessionCallback3 : Glacier2.ISessionCallback
    {
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
            catch (Exception)
            {
                Assert(false);
            }
            finally
            {
                _app.wakeUp();
            }
        }

        public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator() != null);

        private readonly Client _app;
    }

    public class SessionCallback4 : Glacier2.ISessionCallback
    {
        public SessionCallback4(Client app) => _app = app;

        public void Connected(Glacier2.SessionHelper session) => Assert(false);

        public void Disconnected(Glacier2.SessionHelper session) => Assert(false);

        public void ConnectFailed(Glacier2.SessionHelper session, Exception exception)
        {
            try
            {
                throw exception;
            }
            catch (CommunicatorDestroyedException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (FormatException)
            {
                Console.Out.WriteLine("ok");
            }
            catch (Exception)
            {
                Assert(false);
            }
            finally
            {
                _app.wakeUp();
            }
        }

        public void CreatedCommunicator(Glacier2.SessionHelper session) => Assert(session.Communicator() != null);

        private readonly Client _app;
    }

    public override void Run(string[] args)
    {
        System.Collections.Generic.Dictionary<string, string> properties = CreateTestProperties(ref args);
        properties["Ice.Warn.Connections"] = "0";
        properties["Ice.Default.Router"] = $"Glacier2/router:{GetTestEndpoint(properties, 50)}";

        using Communicator communicator = Initialize(properties);

        string transport = GetTestTransport();
        string host = GetTestHost();

        var factory = new Glacier2.SessionFactoryHelper(new SessionCallback1(this), properties);
        Glacier2.SessionHelper? session = null;

        //
        // Test to create a session with wrong userid/password
        //
        lock (this)
        {
            Console.Out.Write("testing SessionHelper connect with wrong userid/password... ");
            Console.Out.Flush();

            factory.SetTransport(transport);
            session = factory.Connect("userid", "xxx");
            while (true)
            {
                try
                {
                    if (!Monitor.Wait(this, 30000))
                    {
                        Assert(false);
                    }
                    break;
                }
                catch (ThreadInterruptedException)
                {
                }
            }
            Assert(!session.IsConnected());
        }

        properties.Remove("Ice.Default.Router");
        factory = new Glacier2.SessionFactoryHelper(new SessionCallback4(this), properties);
        lock (this)
        {
            Console.Out.Write("testing SessionHelper connect interrupt... ");
            Console.Out.Flush();
            factory.SetRouterHost(host);
            factory.SetPort(GetTestPort(1));
            factory.SetTransport(transport);
            session = factory.Connect("userid", "abc123");

            Thread.Sleep(100);
            session.Destroy();

            while (true)
            {
                try
                {
                    if (!Monitor.Wait(this, 30000))
                    {
                        Assert(false);
                    }
                    break;
                }
                catch (ThreadInterruptedException)
                {
                }
            }
            Assert(!session.IsConnected());
        }

        factory = new Glacier2.SessionFactoryHelper(new SessionCallback2(this), properties);
        lock (this)
        {
            Console.Out.Write("testing SessionHelper connect... ");
            Console.Out.Flush();
            factory.SetRouterHost(host);
            factory.SetPort(GetTestPort(50));
            factory.SetTransport(transport);
            session = factory.Connect("userid", "abc123");
            while (true)
            {
                try
                {
                    if (!Monitor.Wait(this, 30000))
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
            Assert(session.IsConnected());
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper categoryForClient after connect... ");
            Console.Out.Flush();
            try
            {
                Assert(!session.CategoryForClient().Equals(""));
            }
            catch (Glacier2.SessionNotExistException)
            {
                Assert(false);
            }
            Console.Out.WriteLine("ok");

            Assert(session.Session() == null);

            Console.Out.Write("testing stringToProxy for server object... ");
            Console.Out.Flush();
            var twoway = ICallbackPrx.Parse($"callback:{GetTestEndpoint(0)}", session.Communicator()!);
            Console.Out.WriteLine("ok");

            Console.Out.Write("pinging server after session creation... ");
            Console.Out.Flush();
            twoway.IcePing();
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            twoway.shutdown();
            Console.Out.WriteLine("ok");

            Assert(session.Communicator() != null);
            Console.Out.Write("testing SessionHelper destroy... ");
            Console.Out.Flush();
            session.Destroy();
            while (true)
            {
                try
                {
                    if (!Monitor.Wait(this, 30000))
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
            Assert(session.IsConnected() == false);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper categoryForClient after destroy... ");
            Console.Out.Flush();
            try
            {
                Assert(!session.CategoryForClient().Equals(""));
                Assert(false);
            }
            catch (Glacier2.SessionNotExistException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper session after destroy... ");
            Assert(session.Session() == null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper communicator after destroy... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("dummy", session.Communicator()!).IcePing();
                Assert(false);
            }
            catch (CommunicatorDestroyedException)
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
                process = IProcessPrx.Parse($"Glacier2/admin -f Process:{GetTestEndpoint(51)}", communicator);
                Console.Out.WriteLine("ok");
            }

            Console.Out.Write("testing Glacier2 shutdown... ");
            process.Shutdown();
            try
            {
                process.IcePing();
                Assert(false);
            }
            catch (System.Exception)
            {
                Console.Out.WriteLine("ok");
            }
        }

        factory = new Glacier2.SessionFactoryHelper(new SessionCallback3(this), properties);
        lock (this)
        {
            Console.Out.Write("testing SessionHelper connect after router shutdown... ");
            Console.Out.Flush();

            factory.SetRouterHost(host);
            factory.SetPort(GetTestPort(50));
            factory.SetTransport(transport);
            session = factory.Connect("userid", "abc123");
            while (true)
            {
                try
                {
                    if (!Monitor.Wait(this, 30000))
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
            Assert(session.IsConnected() == false);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper communicator after connect failure... ");
            Console.Out.Flush();
            try
            {
                IObjectPrx.Parse("dummy", session.Communicator()!).IcePing();
                Assert(false);
            }
            catch (CommunicatorDestroyedException)
            {
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing SessionHelper destroy after connect failure... ");
            Console.Out.Flush();
            session.Destroy();
            Console.Out.WriteLine("ok");
        }
    }

    public void
    wakeUp()
    {
        lock (this)
        {
            Monitor.Pulse(this);
        }
    }

    public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
}
