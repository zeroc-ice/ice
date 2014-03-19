// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Threading;
using System.Collections.Generic;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceSessionClient")]
[assembly: AssemblyDescription("Ice session demo client")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client
{
    public class App : Ice.Application
    {
        class SessionRefreshThread
        {
            public SessionRefreshThread(Ice.Logger logger, int timeout, SessionPrx session)
            {
                _logger = logger;
                _session = session;
                _timeout = timeout;
                _terminated = false;
            }

            public void run()
            {
#if COMPACT
                _m.Lock();
                try
                {
                    while(!_terminated)
                    {
                        _m.TimedWait(_timeout);
                        if(!_terminated)
                        {
                            try
                            {
                                _session.refresh();
                            }
                            catch(Ice.Exception ex)
                            {
                                _logger.warning("SessionRefreshThread: " + ex);
                                _terminated = true;
                            }
                        }
                    }
                }
                finally
                {
                    _m.Unlock();
                }
#else
                lock(this)
                {
                    while(!_terminated)
                    {
                        Monitor.Wait(this, _timeout);
                        if(!_terminated)
                        {
                            try
                            {
                                _session.refresh();
                            }
                            catch(Ice.Exception ex)
                            {
                                _logger.warning("SessionRefreshThread: " + ex);
                                _terminated = true;
                            }
                        }
                    }
                }
#endif
            }

            public void terminate()
            {
#if COMPACT
                _m.Lock();
                try
                {
                    _terminated = true;
                    _m.Notify();
                }
                finally
                {
                    _m.Unlock();
                }
#else
                lock(this)
                {
                    _terminated = true;
                    Monitor.Pulse(this);
                }
#endif
            }

            private Ice.Logger _logger;
            private SessionPrx _session;
            private int _timeout;
            private bool _terminated;
#if COMPACT
            private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
#endif
        }

        public override int run(string[] args)
        {
            if(args.Length > 0)
            {
                Console.Error.WriteLine(appName() + ": too many arguments");
                return 1;
            }

            string name;
            do
            {
                Console.Out.Write("Please enter your name ==> ");
                Console.Out.Flush();
            
                name = Console.In.ReadLine();
                if(name == null)
                {
                    return 1;
                }
                name = name.Trim();
            }
            while(name.Length == 0);

            Ice.ObjectPrx basePrx = communicator().propertyToProxy("SessionFactory.Proxy");
            SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(basePrx);
            if(factory == null)
            {
                Console.Error.WriteLine("invalid proxy");
                return 1;
            }

            SessionPrx session = factory.create(name);

            SessionRefreshThread refresh = new SessionRefreshThread(communicator().getLogger(), 5000, session);
            Thread refreshThread = new Thread(new ThreadStart(refresh.run));
            refreshThread.Start();

            List<HelloPrx> hellos = new List<HelloPrx>();

            menu();

            try
            {
                bool destroy = true;
                bool shutdown = false;
                while(true)
                {
                    Console.Out.Write("==> ");
                    Console.Out.Flush();
                    string line = Console.In.ReadLine();
                    if(line == null)
                    {
                        break;
                    }
                    if(line.Length > 0 && Char.IsDigit(line[0]))
                    {
                        int index = Int32.Parse(line);
                        if(index < hellos.Count)
                        {
                            HelloPrx hello = hellos[index];
                            hello.sayHello();
                        }
                        else
                        {
                            Console.Out.WriteLine("Index is too high. " + hellos.Count +
                                                  " hello objects exist so far.\n" +
                                                  "Use `c' to create a new hello object.");
                        }
                    }
                    else if(line.Equals("c"))
                    {
                        hellos.Add(session.createHello());
                        Console.Out.WriteLine("Created hello object " + (hellos.Count - 1));
                    }
                    else if(line.Equals("s"))
                    {
                        destroy = false;
                        shutdown = true;
                        break;
                    }
                    else if(line.Equals("x"))
                    {
                        break;
                    }
                    else if(line.Equals("t"))
                    {
                        destroy = false;
                        break;
                    }
                    else if(line.Equals("?"))
                    {
                        menu();
                    }
                    else
                    {
                        Console.Out.WriteLine("Unknown command `" + line + "'.");
                        menu();
                    }
                }

                //
                // The refresher thread must be terminated before destroy is
                // called, otherwise it might get ObjectNotExistException. refresh
                // is set to 0 so that if session.destroy() raises an exception
                // the thread will not be re-terminated and re-joined.
                //
                refresh.terminate();
                refreshThread.Join();
                refresh = null;

                if(destroy)
                {
                    session.destroy();
                }
                if(shutdown)
                {
                    factory.shutdown();
                }
            }
            catch(System.Exception)
            {
                //
                // The refresher thread must be terminated in the event of a
                // failure.
                //
                if(refresh != null)
                {
                    refresh.terminate();
                    refreshThread.Join();
                    refresh = null;
                }
                throw;
            }

            return 0;
        }

        private static void menu()
        {
            Console.Out.WriteLine(
                "usage:\n" +
                "c:     create a new per-client hello object\n" +
                "0-9:   send a greeting to a hello object\n" +
                "s:     shutdown the server and exit\n" +
                "x:     exit\n" +
                "t:     exit without destroying the session\n" +
                "?:     help\n");
        }
    }

    public static int Main(string[] args)
    {
        App app = new App();
        return app.main(args, "config.client");
    }
}
