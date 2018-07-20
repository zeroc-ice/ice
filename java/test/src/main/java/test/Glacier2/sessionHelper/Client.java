// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.sessionHelper;

import java.io.PrintWriter;
import test.Glacier2.sessionHelper.Test.CallbackPrx;

import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

public class Client extends test.TestHelper
{
    public Client()
    {
        out = getWriter();
        me = this;
    }

    public void run(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" +
                                        getTestEndpoint(initData.properties, 50));
        initData.dispatcher = (Runnable runnable, com.zeroc.Ice.Connection c) ->
            {
                _workQueue.submit(runnable);
            };

        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            String protocol = getTestProtocol();
            String host = getTestHost();

            _factory = new com.zeroc.Glacier2.SessionFactoryHelper(initData, new com.zeroc.Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(com.zeroc.Glacier2.SessionHelper session)
                    throws com.zeroc.Glacier2.SessionNotExistException
                    {
                        test(false);
                    }

                    @Override
                    public void disconnected(com.zeroc.Glacier2.SessionHelper session)
                    {
                        test(false);
                    }

                    @Override
                    public void connectFailed(com.zeroc.Glacier2.SessionHelper session, Throwable exception)
                    {
                        try
                        {
                            throw exception;
                        }
                        catch(com.zeroc.Glacier2.PermissionDeniedException ex)
                        {
                            out.println("ok");
                            synchronized(test.Glacier2.sessionHelper.Client.this)
                            {
                                test.Glacier2.sessionHelper.Client.wakeUp();
                            }
                        }
                        catch(Throwable ex)
                        {
                            ex.printStackTrace();
                            test(false);
                        }
                    }

                    @Override
                    public void createdCommunicator(com.zeroc.Glacier2.SessionHelper session)
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
                        wait(30000);
                        break;
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                test(!_session.isConnected());
            }

            initData.properties.setProperty("Ice.Default.Router", "");
            _factory = new com.zeroc.Glacier2.SessionFactoryHelper(initData, new com.zeroc.Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(com.zeroc.Glacier2.SessionHelper session)
                    throws com.zeroc.Glacier2.SessionNotExistException
                    {
                        test(false);
                    }

                    @Override
                    public void disconnected(com.zeroc.Glacier2.SessionHelper session)
                    {
                        test(false);
                    }

                    @Override
                    public void connectFailed(com.zeroc.Glacier2.SessionHelper session, Throwable exception)
                    {
                        try
                        {
                            throw exception;
                        }
                        catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
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
                    public void createdCommunicator(com.zeroc.Glacier2.SessionHelper session)
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
                        wait(30000);
                        break;
                    }
                    catch(java.lang.InterruptedException ex)
                    {
                    }
                }
                test(!_session.isConnected());
            };

            _factory = new com.zeroc.Glacier2.SessionFactoryHelper(initData, new com.zeroc.Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(com.zeroc.Glacier2.SessionHelper session)
                    throws com.zeroc.Glacier2.SessionNotExistException
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.wakeUp();
                        }
                    }

                    @Override
                    public void disconnected(com.zeroc.Glacier2.SessionHelper session)
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.wakeUp();
                        }
                    }

                    @Override
                    public void connectFailed(com.zeroc.Glacier2.SessionHelper session, Throwable ex)
                    {
                        test(false);
                    }

                    @Override
                    public void createdCommunicator(com.zeroc.Glacier2.SessionHelper session)
                    {
                        test(session.communicator() != null);
                    }
                });

            synchronized(this)
            {
                out.print("testing SessionHelper connect... ");
                out.flush();
                _factory.setRouterHost(host);
                _factory.setPort(getTestPort(50));
                _factory.setProtocol(protocol);
                _session = _factory.connect("userid", "abc123");
                while(true)
                {
                    try
                    {
                        wait(30000);
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
                catch(com.zeroc.Glacier2.SessionNotExistException ex)
                {
                    test(false);
                }
                out.println("ok");

                test(_session.session() == null);

                out.print("testing stringToProxy for server object... ");
                out.flush();
                com.zeroc.Ice.ObjectPrx base = _session.communicator().stringToProxy("callback:" + getTestEndpoint(0));
                out.println("ok");

                out.print("pinging server after session creation... ");
                out.flush();
                base.ice_ping();
                out.println("ok");

                out.print("testing checked cast for server object... ");
                out.flush();
                CallbackPrx twoway = CallbackPrx.checkedCast(base);
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
                catch(com.zeroc.Glacier2.SessionNotExistException ex)
                {
                }
                out.println("ok");

                out.print("testing SessionHelper session after destroy... ");
                test(_session.session() == null);
                out.println("ok");

                out.print("testing SessionHelper communicator after destroy... ");
                out.flush();
                try
                {
                    test(_session.communicator() != null);
                    _session.communicator().stringToProxy("dummy");
                    test(false);
                }
                catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
                {
                }
                out.println("ok");

                out.print("uninstalling router with communicator... ");
                out.flush();
                communicator.setDefaultRouter(null);
                out.println("ok");

                com.zeroc.Ice.ObjectPrx processBase;
                {
                    out.print("testing stringToProxy for process object... ");
                    processBase = communicator.stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
                    out.println("ok");
                }

                com.zeroc.Ice.ProcessPrx process;
                {
                    out.print("testing checked cast for admin object... ");
                    process = com.zeroc.Ice.ProcessPrx.checkedCast(processBase);
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
                catch(com.zeroc.Ice.LocalException ex)
                {
                    out.println("ok");
                }
            }

            _factory = new com.zeroc.Glacier2.SessionFactoryHelper(initData, new com.zeroc.Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(com.zeroc.Glacier2.SessionHelper session)
                    throws com.zeroc.Glacier2.SessionNotExistException
                    {
                        test(false);
                    }

                    @Override
                    public void disconnected(com.zeroc.Glacier2.SessionHelper session)
                    {
                        test(false);
                    }

                    @Override
                    public void connectFailed(com.zeroc.Glacier2.SessionHelper session, Throwable exception)
                    {
                        try
                        {
                            throw exception;
                        }
                        catch(com.zeroc.Ice.ConnectFailedException ex)
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
                    public void createdCommunicator(com.zeroc.Glacier2.SessionHelper session)
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
                _factory.setPort(getTestPort(50));
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
                catch(com.zeroc.Ice.CommunicatorDestroyedException ex)
                {
                }
                out.println("ok");

                out.print("testing SessionHelper destroy after connect failure... ");
                out.flush();
                _session.destroy();
                out.println("ok");
            }
            _workQueue.shutdown();
        }
    }

    public static void wakeUp()
    {
        me.notify();
    }

    private com.zeroc.Glacier2.SessionHelper _session;
    private com.zeroc.Glacier2.SessionFactoryHelper _factory;
    private ExecutorService _workQueue = Executors.newSingleThreadExecutor();
    static public Client me;
    final public PrintWriter out;
}
