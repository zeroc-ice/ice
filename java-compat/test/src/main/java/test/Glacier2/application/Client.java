// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.application;

import java.io.PrintWriter;
import test.Glacier2.application.Test.CallbackPrx;
import test.Glacier2.application.Test.CallbackPrxHelper;
import test.Glacier2.application.Test.CallbackReceiverPrx;
import test.Glacier2.application.Test.CallbackReceiverPrxHelper;
import test.Glacier2.application.Test._CallbackReceiverDisp;

public class Client extends test.TestHelper
{
    public Client()
    {
        out = getWriter();
    }

    class CallbackReceiverI extends test.Glacier2.application.Test._CallbackReceiverDisp
    {
        public synchronized void
        callback(Ice.Current current)
        {
            _received = true;
            notify();
        }

        public synchronized void
        waitForCallback()
        {
            while(!_received)
            {
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                    continue;
                }
            }
            _received = false;
        }

        boolean _received = false;
    }

    class Application extends Glacier2.Application
    {
        public Application()
        {
            _receiver = new CallbackReceiverI();
        }

        @Override
        public Glacier2.SessionPrx
        createSession()
        {
            Glacier2.SessionPrx session = null;
            try
            {
                session = Glacier2.SessionPrxHelper.uncheckedCast(router().createSession("userid", "abc123"));
            }
            catch(Glacier2.PermissionDeniedException ex)
            {
                System.out.println("permission denied:\n" + ex.reason);
            }
            catch(Glacier2.CannotCreateSessionException ex)
            {
                System.out.println("cannot create session:\n" + ex.reason);
            }
            return session;
        }

        @Override
        public int runWithSession(String[] args) throws RestartSessionException
        {
            try
            {
                test(router() != null);
                test(categoryForClient() != "");
                test(objectAdapter() != null);

                if(_restart == 0)
                {
                    out.print("testing Glacier2::Application restart... ");
                    out.flush();
                }
                Ice.ObjectPrx base = communicator().stringToProxy("callback:" +
                                                                  getTestEndpoint(communicator().getProperties(), 0));
                CallbackPrx callback = CallbackPrxHelper.uncheckedCast(base);
                if(++_restart < 5)
                {
                    CallbackReceiverPrx receiver = CallbackReceiverPrxHelper.uncheckedCast(addWithUUID(_receiver));
                    callback.initiateCallback(receiver);
                    _receiver.waitForCallback();
                    restart();
                }
                out.println("ok");

                out.print("testing server shutdown... ");
                out.flush();
                callback.shutdown();
                out.println("ok");

            }
            catch(Glacier2.SessionNotExistException ex)
            {
            }

            return 0;
        }

        @Override
        public void sessionDestroyed()
        {
            _destroyed = true;
        }

        public int _restart = 0;
        public boolean _destroyed = false;
        private CallbackReceiverI _receiver;
    }

    public void run(String[] args)
    {
        Application app = new Application();

        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Warn.Connections", "0");

        try(Ice.Communicator communicator = initialize(args))
        {
            properties.setProperty("Ice.Default.Router", "Glacier2/router:" + getTestEndpoint(50));

            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = properties._clone();

            int status = app.main("Client", args, initData);
            if(status != 0)
            {
                test(false);
            }

            out.print("testing stringToProxy for process object... ");
            out.flush();
            Ice.ObjectPrx processBase = communicator.stringToProxy("Glacier2/admin -f Process:" + getTestEndpoint(51));
            out.println("ok");

            out.print("testing checked cast for admin object... ");
            out.flush();
            Ice.ProcessPrx process = Ice.ProcessPrxHelper.checkedCast(processBase);
            test(process != null);
            out.println("ok");

            out.print("testing Glacier2 shutdown... ");
            out.flush();
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

            test(app._restart == 5);
            test(app._destroyed);
        }
    }

    final public PrintWriter out;
}
