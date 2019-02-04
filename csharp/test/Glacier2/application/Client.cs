//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Reflection;
using System.Threading;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    class CallbackReceiverI : Test.CallbackReceiverDisp_
    {
        public override void
        callback(Ice.Current current)
        {
            lock(this)
            {
                _received = true;
                Monitor.PulseAll(this);
            }
        }

        public void
        waitForCallback()
        {
            lock(this)
            {
                while(!_received)
                {
                    try
                    {
                        Monitor.Wait(this);
                    }
                    catch(ThreadInterruptedException)
                    {
                        continue;
                    }
                }
                _received = false;
            }
        }

        bool _received = false;
    }

    class Application : Glacier2.Application
    {
        public Application()
        {
            _receiver = new CallbackReceiverI();
        }

        public override Glacier2.SessionPrx
        createSession()
        {
            Glacier2.SessionPrx session = null;
            try
            {
                session = Glacier2.SessionPrxHelper.uncheckedCast(router().createSession("userid", "abc123"));
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                Console.WriteLine("permission denied:\n" + ex.reason);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                Console.WriteLine("cannot create session:\n" + ex.reason);
            }
            return session;
        }

        public override int runWithSession(string[] args)
        {
            test(router() != null);
            test(categoryForClient() != "");
            test(objectAdapter() != null);

            if(_restart == 0)
            {
                Console.Out.Write("testing Glacier2::Application restart... ");
                Console.Out.Flush();
            }
            Ice.ObjectPrx @base = communicator().stringToProxy("callback:" +
                                                                getTestEndpoint(communicator().getProperties(), 0));
            CallbackPrx callback = CallbackPrxHelper.uncheckedCast(@base);
            if(++_restart < 5)
            {
                CallbackReceiverPrx receiver = CallbackReceiverPrxHelper.uncheckedCast(addWithUUID(_receiver));
                callback.initiateCallback(receiver);
                _receiver.waitForCallback();
                restart();
            }
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing server shutdown... ");
            Console.Out.Flush();
            callback.shutdown();
            Console.Out.WriteLine("ok");
            return 0;
        }

        public override void sessionDestroyed()
        {
            _destroyed = true;
        }

        public int _restart = 0;
        public bool _destroyed = false;
        private CallbackReceiverI _receiver;
    }

    public override void run(string[] args)
    {
        Application app = new Application();

        Ice.Properties properties = createTestProperties(ref args);
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = properties.ice_clone_();
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Router",
                                        "Glacier2/router:" + getTestEndpoint(initData.properties, 50));
        if(app.main(args, initData) != 0)
        {
            test(false);
        }

        using(var communicator = initialize(properties))
        {
            Console.Out.Write("testing stringToProxy for process object... ");
            Console.Out.Flush();
            Ice.ObjectPrx processBase = communicator.stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing checked cast for admin object... ");
            Console.Out.Flush();
            Ice.ProcessPrx process = Ice.ProcessPrxHelper.checkedCast(processBase);
            test(process != null);
            Console.Out.WriteLine("ok");

            Console.Out.Write("testing Glacier2 shutdown... ");
            Console.Out.Flush();
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
            test(app._restart == 5);
            test(app._destroyed);
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
