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
import test.Glacier2.sessionHelper.Test.CallbackPrxHelper;

import java.util.concurrent.Executors;
import java.util.concurrent.ExecutorService;

public class Client extends test.TestHelper
{
    public void run(String[] args)
    {
        final ExecutorService workQueue = Executors.newSingleThreadExecutor();
        final PrintWriter out = getWriter();
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" +
                                        getTestEndpoint(initData.properties, 50));
        initData.dispatcher = new Ice.Dispatcher()
            {
                @Override
                public void dispatch(Runnable runnable, Ice.Connection connection)
                {
                    workQueue.submit(runnable);
                }
            };

        try(Ice.Communicator communicator = initialize(initData))
        {
            String protocol = getTestProtocol();
            String host = getTestHost();

            Glacier2.SessionFactoryHelper factory =
                new Glacier2.SessionFactoryHelper(initData, new Glacier2.SessionCallback()
                    {
                        @Override
                        public void connected(Glacier2.SessionHelper session) throws Glacier2.SessionNotExistException
                        {
                            test(false);
                        }

                        @Override
                        public void disconnected(Glacier2.SessionHelper session)
                        {
                            test(false);
                        }

                        @Override
                        public void connectFailed(Glacier2.SessionHelper session, Throwable exception)
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
                                    test.Glacier2.sessionHelper.Client.this.notify();
                                }
                            }
                            catch(Throwable ex)
                            {
                                test(false);
                            }
                        }

                        @Override
                        public void createdCommunicator(Glacier2.SessionHelper session)
                        {
                            test(session.communicator() != null);
                        }
                    });

            Glacier2.SessionHelper session;
            //
            // Test to create a session with wrong userid/password
            //
            synchronized(this)
            {
                out.print("testing SessionHelper connect with wrong userid/password... ");
                out.flush();

                session = factory.connect("userid", "xxx");
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
                test(!session.isConnected());
            }

            initData.properties.setProperty("Ice.Default.Router", "");
            factory = new Glacier2.SessionFactoryHelper(initData, new Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(Glacier2.SessionHelper session) throws Glacier2.SessionNotExistException
                    {
                        test(false);
                    }

                    @Override
                    public void disconnected(Glacier2.SessionHelper session)
                    {
                        test(false);
                    }

                    @Override
                    public void connectFailed(Glacier2.SessionHelper session, Throwable exception)
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
                                test.Glacier2.sessionHelper.Client.this.notify();
                            }
                        }
                        catch(Throwable ex)
                        {
                            test(false);
                        }
                    }

                    @Override
                    public void createdCommunicator(Glacier2.SessionHelper session)
                    {
                        test(session.communicator() != null);
                    }
                });

            synchronized(this)
            {
                out.print("testing SessionHelper connect interrupt... ");
                out.flush();
                factory.setRouterHost(host);
                factory.setPort(getTestPort(1));
                factory.setProtocol(protocol);
                session = factory.connect("userid", "abc123");

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
                session.destroy();

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
                test(!session.isConnected());
            };

            factory = new Glacier2.SessionFactoryHelper(initData, new Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(Glacier2.SessionHelper session) throws Glacier2.SessionNotExistException
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.this.notify();
                        }
                    }

                    @Override
                    public void disconnected(Glacier2.SessionHelper session)
                    {
                        out.println("ok");
                        synchronized(test.Glacier2.sessionHelper.Client.this)
                        {
                            test.Glacier2.sessionHelper.Client.this.notify();
                        }
                    }

                    @Override
                    public void connectFailed(Glacier2.SessionHelper session, Throwable ex)
                    {
                        test(false);
                    }

                    @Override
                    public void createdCommunicator(Glacier2.SessionHelper session)
                    {
                        test(session.communicator() != null);
                    }
                });

            synchronized(this)
            {
                out.print("testing SessionHelper connect... ");
                out.flush();
                factory.setRouterHost(host);
                factory.setPort(getTestPort(50));
                factory.setProtocol(protocol);
                session = factory.connect("userid", "abc123");
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
                test(session.isConnected());
                out.println("ok");

                out.print("testing SessionHelper categoryForClient after connect... ");
                out.flush();
                try
                {
                    test(!session.categoryForClient().equals(""));
                }
                catch(Glacier2.SessionNotExistException ex)
                {
                    test(false);
                }
                out.println("ok");

                test(session.session() == null);

                out.print("testing stringToProxy for server object... ");
                out.flush();
                Ice.ObjectPrx base = session.communicator().stringToProxy("callback:" + getTestEndpoint(0));
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

                test(session.communicator() != null);
                out.print("testing SessionHelper destroy... ");
                out.flush();
                session.destroy();
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
                test(session.isConnected() == false);
                out.println("ok");

                out.print("testing SessionHelper categoryForClient after destroy... ");
                out.flush();
                try
                {
                    test(!session.categoryForClient().equals(""));
                    test(false);
                }
                catch(Glacier2.SessionNotExistException ex)
                {
                }
                out.println("ok");

                out.print("testing SessionHelper session after destroy... ");
                test(session.session() == null);
                out.println("ok");

                out.print("testing SessionHelper communicator after destroy... ");
                out.flush();
                try
                {
                    test(session.communicator() != null);
                    session.communicator().stringToProxy("dummy");
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
                    processBase = communicator().stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
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

            factory = new Glacier2.SessionFactoryHelper(initData, new Glacier2.SessionCallback()
                {
                    @Override
                    public void connected(Glacier2.SessionHelper session) throws Glacier2.SessionNotExistException
                    {
                        test(false);
                    }

                    @Override
                    public void disconnected(Glacier2.SessionHelper session)
                    {
                        test(false);
                    }

                    @Override
                    public void connectFailed(Glacier2.SessionHelper session, Throwable exception)
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
                                test.Glacier2.sessionHelper.Client.this.notify();
                            }
                        }
                        catch(Throwable ex)
                        {
                            test(false);
                        }
                    }

                    @Override
                    public void createdCommunicator(Glacier2.SessionHelper session)
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

                factory.setRouterHost(host);
                factory.setPort(getTestPort(50));
                factory.setProtocol(protocol);
                session = factory.connect("userid", "abc123");
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
                test(session.isConnected() == false);
                out.println("ok");

                out.print("testing SessionHelper communicator after connect failure... ");
                out.flush();
                try
                {
                    test(session.communicator() != null);
                    session.communicator().stringToProxy("dummy");
                    test(false);
                }
                catch(Ice.CommunicatorDestroyedException ex)
                {
                }
                out.println("ok");

                out.print("testing SessionHelper destroy after connect failure... ");
                out.flush();
                session.destroy();
                out.println("ok");
            }
        }
        workQueue.shutdown();
    }
}
