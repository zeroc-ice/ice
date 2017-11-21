// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        _initData = base.getInitData(ref args);
        _initData.properties.setProperty("Ice.Warn.Connections", "0");
        return _initData;
    }

    public override int run(string[] args)
    {
        Application app = new Application();
        _initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" +
                                            getTestEndpoint(_initData.properties, 50));
        int status = app.main(args, _initData);

        Console.Out.Write("testing stringToProxy for process object... ");
        Console.Out.Flush();
        Ice.ObjectPrx processBase = communicator().stringToProxy("Glacier2/admin -f Process:" +
                                                                 getTestEndpoint(communicator().getProperties(), 51));
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

        return status;
    }

    public static int Main(string[] args)
    {
        Client app = new Client();
        return app.runmain(args);
    }

    private Ice.InitializationData _initData;
}
