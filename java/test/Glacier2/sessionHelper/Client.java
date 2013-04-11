// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.sessionHelper;

import javax.swing.SwingUtilities;
import java.io.PrintWriter;
import java.io.StringWriter;

import test.Glacier2.sessionHelper.Test.CallbackException;
import test.Glacier2.sessionHelper.Test.CallbackPrx;
import test.Glacier2.sessionHelper.Test.CallbackPrxHelper;
import test.Glacier2.sessionHelper.Test.CallbackReceiverPrx;
import test.Glacier2.sessionHelper.Test.CallbackReceiverPrxHelper;

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

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        _initData = new Ice.InitializationData();
        _initData.properties = Ice.Util.createProperties(argsH);
        _initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:default -p 12347");
        _initData.dispatcher = new Ice.Dispatcher()
            {
                public void
                dispatch(Runnable runnable, Ice.Connection connection)
                {
                    SwingUtilities.invokeLater(runnable);
                }
            };

        return _initData;
    }

    public int run(String[] args)
    {
        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                     test(false);
                }

                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    test(false);
                }

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

            _factory.setRouterHost("127.0.0.1");
            _factory.setPort(12347);
            _factory.setRouterIdentity(Ice.Util.stringToIdentity("Glacier2/router"));
            _factory.setSecure(false);
            _session = _factory.connect("userid", "xxx");
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
        }

        _factory = new Glacier2.SessionFactoryHelper(_initData, new Glacier2.SessionCallback()
            {
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

                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    out.println("ok");
                    synchronized(test.Glacier2.sessionHelper.Client.this)
                    {
                        test.Glacier2.sessionHelper.Client.wakeUp();
                    }
                }

                public void
                connectFailed(Glacier2.SessionHelper session, Throwable ex)
                {
                    test(false);
                }

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
            _factory.setRouterHost("127.0.0.1");
            _factory.setPort(12347);
            _factory.setRouterIdentity(Ice.Util.stringToIdentity("Glacier2/router"));
            _factory.setSecure(false);
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
            Ice.ObjectPrx base = _session.communicator().stringToProxy("callback:tcp -p 12010");
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
                Glacier2.SessionPrx session = _session.session();
                test(false);
            }
            catch(Glacier2.SessionNotExistException ex)
            {
            }
            out.println("ok");

            out.print("testing SessionHelper communicator after destroy... ");
            out.flush();
            test(_session.communicator() == null);
            out.println("ok");


            out.print("uninstalling router with communicator... ");
            out.flush();
            communicator().setDefaultRouter(null);
            out.println("ok");

            Ice.ObjectPrx processBase;
            {
                out.print("testing stringToProxy for process object... ");
                processBase = communicator().stringToProxy("Glacier2/admin -f Process:tcp -h 127.0.0.1 -p 12348");
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
                public void
                connected(Glacier2.SessionHelper session)
                    throws Glacier2.SessionNotExistException
                {
                     test(false);
                }

                public void
                disconnected(Glacier2.SessionHelper session)
                {
                    test(false);
                }

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

            _factory.setRouterHost("127.0.0.1");
            _factory.setPort(12347);
            _factory.setRouterIdentity(Ice.Util.stringToIdentity("Glacier2/router"));
            _factory.setSecure(false);
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
            test(_session.communicator() == null);
            out.println("ok");

            out.print("testing SessionHelper destroy after connect failure... ");
            out.flush();
            _session.destroy();
            out.println("ok");
        }

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
    static public Client me;
    final public PrintWriter out;
}
