// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.sessionHelper;

import java.io.PrintWriter;
import test.Glacier2.sessionHelper.Test.CallbackPrx;
import test.Glacier2.sessionHelper.Test.CallbackPrxHelper;

public class Client extends test.Util.Application
{
    Client()
    {
        out = getWriter();
        me = this;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    public class WorkQueue extends Thread
    {
        @Override
        public synchronized void
        run()
        {
            while(!_done)
            {
                if(_runnables.size() == 0)
                {
                    try
                    {
                        wait();
                        if(_done)
                        {
                            break;
                        }
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }

                if(_runnables.size() != 0)
                {
                    _runnables.pop().run();
                }
            }
        }

        public synchronized void
        add(Runnable runnable)
        {
            if(!_done)
            {
                if(_runnables.size() == 0)
                {
                    notify();
                }
                _runnables.add(runnable);
            }
        }

        public synchronized void
        _destroy()                  // Thread.destroy is deprecated.
        {
            _done = true;
            notify();
        }

        private java.util.LinkedList<Runnable> _runnables = new java.util.LinkedList<Runnable>();
        private boolean _done = false;
    }

    @Override
    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        _initData = super.getInitData(argsH);
        _initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" +
                                         getTestEndpoint(_initData.properties, 10));
        _initData.dispatcher = new Ice.Dispatcher()
            {
                @Override
                public void
                dispatch(Runnable runnable, Ice.Connection connection)
                {
                    _workQueue.add(runnable);
                }
            };

        return _initData;
    }

    @Override
    public int run(String[] args)
    {
        String protocol = getTestProtocol();
        String host = getTestHost();
        _workQueue = new WorkQueue();
        _workQueue.start();

        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
                @Override
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                     test(false);
                }

                @Override
                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    test(false);
                }

                @Override
                public void
                connectFailed(Glacier2.SessionHelper session, Throwable exception)
                {
                    try
                    {
                        throw exception;
                    }
                    catch(Glacier2.PermissionDeniedException ex)
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.wakeUp();
                        }
                    }
                    catch(Throwable ex)
                    {
                        test(false);
                    }
                }

                @Override
                public void
                createdCommunicator(Glacier2.SessionHelper session)
                {
                    test(session.communicator() != null);
                }
            });

        //
        // Test to create a session with wrong userid/password
        //
        synchronized(this)
        {
            out.print("testing SessionHelper connect with wrong userid/password... ");
            out.flush();

            _session = _factory.connect("userid", "xxx");
            while(true)
            {
                try
                {
                    wait(2000);
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            test(!_session.isConnected());
        }

        _initData.properties.setProperty("Ice.Default.Router", "");
        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
                @Override
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                    test(false);
                }

                @Override
                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    test(false);
                }

                @Override
                public void
                connectFailed(Glacier2.SessionHelper session, Throwable exception)
                {
                    try
                    {
                        throw exception;
                    }
                    catch(Ice.CommunicatorDestroyedException ex)
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.wakeUp();
                        }
                    }
                    catch(Throwable ex)
                    {
                        test(false);
                    }
                }

                @Override
                public void
                createdCommunicator(Glacier2.SessionHelper session)
                {
                    test(session.communicator() != null);
                }
            });

        synchronized(this)
        {
            out.print("testing SessionHelper connect interrupt... ");
            out.flush();
            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(1));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");

            while(true)
            {
                try
                {
                    Thread.sleep(100);
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            _session.destroy();

            while(true)
            {
                try
                {
                    wait(2000);
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }
            test(!_session.isConnected());
        };

        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
                @Override
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                    out.println("ok");
                    synchronized(test.Glacier2.sessionHelper.Client.this)
                    {
                        test.Glacier2.sessionHelper.Client.wakeUp();
                    }
                }

                @Override
                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    out.println("ok");
                    synchronized(test.Glacier2.sessionHelper.Client.this)
                    {
                        test.Glacier2.sessionHelper.Client.wakeUp();
                    }
                }

                @Override
                public void
                connectFailed(Glacier2.SessionHelper session, Throwable ex)
                {
                    test(false);
                }

                @Override
                public void
                createdCommunicator(Glacier2.SessionHelper session)
                {
                    test(session.communicator() != null);
                }
            });

        synchronized(this)
        {
            out.print("testing SessionHelper connect... ");
            out.flush();
            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(10));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");
            while(true)
            {
                try
                {
                    wait(2000);
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            out.print("testing SessionHelper isConnected after connect... ");
            out.flush();
            test(_session.isConnected());
            out.println("ok");

            out.print("testing SessionHelper categoryForClient after connect... ");
            out.flush();
            try
            {
                test(!_session.categoryForClient().equals(""));
            }
            catch(Glacier2.SessionNotExistException ex)
            {
                test(false);
            }
            out.println("ok");

//             try
//             {
//                 test(_session.session() != null);
//             }
//             catch(Glacier2.SessionNotExistException ex)
//             {
//                 test(false);
//             }

            out.print("testing stringToProxy for server object... ");
            out.flush();
            Ice.ObjectPrx base = _session.communicator().stringToProxy("callback:" + getTestEndpoint(0));
            out.println("ok");

            out.print("pinging server after session creation... ");
            out.flush();
            base.ice_ping();
            out.println("ok");

            out.print("testing checked cast for server object... ");
            out.flush();
            CallbackPrx twoway = CallbackPrxHelper.checkedCast(base);
            test(twoway != null);
            out.println("ok");

            out.print("testing server shutdown... ");
            out.flush();
            twoway.shutdown();
            out.println("ok");

            test(_session.communicator() != null);
            out.print("testing SessionHelper destroy... ");
            out.flush();
            _session.destroy();
            while(true)
            {
                try
                {
                    wait();
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            out.print("testing SessionHelper isConnected after destroy... ");
            out.flush();
            test(_session.isConnected() == false);
            out.println("ok");

            out.print("testing SessionHelper categoryForClient after destroy... ");
            out.flush();
            try
            {
                test(!_session.categoryForClient().equals(""));
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
            }
            out.println("ok");

            out.print("testing SessionHelper session after destroy... ");
            try
            {
                _session.session();
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
            }
            out.println("ok");

            out.print("testing SessionHelper communicator after destroy... ");
            out.flush();
            try
            {
                test(_session.communicator() != null);
                _session.communicator().stringToProxy("dummy");
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
            }
            out.println("ok");


            out.print("uninstalling router with communicator... ");
            out.flush();
            communicator().setDefaultRouter(null);
            out.println("ok");

            Ice.ObjectPrx processBase;
            {
                out.print("testing stringToProxy for process object... ");
                processBase = communicator().stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(11));
                out.println("ok");
            }


            Ice.ProcessPrx process;
            {
                out.print("testing checked cast for admin object... ");
                process = Ice.ProcessPrxHelper.checkedCast(processBase);
                test(process != null);
                out.println("ok");
            }

            out.print("testing Glacier2 shutdown... ");
            process.shutdown();
            try
            {
                process.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
                out.println("ok");
            }
        }

        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
                @Override
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                     test(false);
                }

                @Override
                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    test(false);
                }

                @Override
                public void
                connectFailed(Glacier2.SessionHelper session, Throwable exception)
                {
                    try
                    {
                        throw exception;
                    }
                    catch(Ice.ConnectFailedException ex)
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.wakeUp();
                        }
                    }
                    catch(Throwable ex)
                    {
                        test(false);
                    }
                }

                @Override
                public void
                createdCommunicator(Glacier2.SessionHelper session)
                {
                    test(session.communicator() != null);
                }
            });

        //
        // Wait a bit to ensure glaci2router has been shutdown.
        //
        while(true)
        {
            try
            {
                Thread.sleep(100);
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }

        synchronized(this)
        {
            out.print("testing SessionHelper connect after router shutdown... ");
            out.flush();

            _factory.setRouterHost(host);
            _factory.setPort(getTestPort(10));
            _factory.setProtocol(protocol);
            _session = _factory.connect("userid", "abc123");
            while(true)
            {
                try
                {
                    wait();
                    break;
                }
                catch(java.lang.InterruptedException ex)
                {
                }
            }

            out.print("testing SessionHelper isConnect after connect failure... ");
            out.flush();
            test(_session.isConnected() == false);
            out.println("ok");

            out.print("testing SessionHelper communicator after connect failure... ");
            out.flush();
            try
            {
                test(_session.communicator() != null);
                _session.communicator().stringToProxy("dummy");
                test(false);
            }
            catch(Ice.CommunicatorDestroyedException ex)
            {
            }
            out.println("ok");

            out.print("testing SessionHelper destroy after connect failure... ");
            out.flush();
            _session.destroy();
            out.println("ok");
        }

        _workQueue._destroy();

        return 0;
    }

    public static void
    wakeUp()
    {
        me.notify();
    }

    public static void
    main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }

    private Glacier2.SessionHelper _session;
    private Glacier2.SessionFactoryHelper _factory;
    private Ice.InitializationData _initData;
    private WorkQueue _workQueue;
    static public Client me;
    final public PrintWriter out;
}
