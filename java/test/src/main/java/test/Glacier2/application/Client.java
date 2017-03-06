// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Glacier2.application;

import java.io.PrintWriter;
import test.Glacier2.application.Test.CallbackPrx;
import test.Glacier2.application.Test.CallbackReceiverPrx;

public class Client extends test.Util.Application
{
    Client()
    {
        out = getWriter();
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }
    
    class CallbackReceiverI implements test.Glacier2.application.Test.CallbackReceiver
    {
        @Override
        public synchronized void 
        callback(com.zeroc.Ice.Current current)
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
    
    class Application extends com.zeroc.Glacier2.Application
    {
        public Application()
        {
            _receiver = new CallbackReceiverI();
        }

        @Override
        public com.zeroc.Glacier2.SessionPrx
        createSession()
        {
            com.zeroc.Glacier2.SessionPrx session = null;
            try
            {
                session = com.zeroc.Glacier2.SessionPrx.uncheckedCast(router().createSession("userid", "abc123"));
            }
            catch(com.zeroc.Glacier2.PermissionDeniedException ex)
            {
                System.out.println("permission denied:\n" + ex.reason);
            }
            catch(com.zeroc.Glacier2.CannotCreateSessionException ex)
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
                com.zeroc.Ice.ObjectPrx base = communicator().stringToProxy("callback:" + getTestEndpoint(communicator().getProperties(), 0));
                CallbackPrx callback = CallbackPrx.uncheckedCast(base);
                if(++_restart < 5)
                {
                    CallbackReceiverPrx receiver = CallbackReceiverPrx.uncheckedCast(addWithUUID(_receiver));
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
            catch(com.zeroc.Glacier2.SessionNotExistException ex)
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
    
    public int run(String[] args)
    {
        Application app = new Application();
        
        java.util.List<String> remainingArgs = new java.util.ArrayList<>();
        com.zeroc.Ice.InitializationData initData = getInitData(args, remainingArgs);
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Router", "Glacier2/router:" + getTestEndpoint(initData.properties, 10));
        
        int status = app.main("Client", args, initData);
        
        initData.properties.setProperty("Ice.Default.Router", "");
        com.zeroc.Ice.Communicator communicator = com.zeroc.Ice.Util.initialize(initData);
    
        out.print("testing stringToProxy for process object... ");
        out.flush();
        com.zeroc.Ice.ObjectPrx processBase = communicator.stringToProxy("Glacier2/admin -f Process:" +
                                                                    getTestEndpoint(communicator.getProperties(), 11));
        out.println("ok");

        out.print("testing checked cast for admin object... ");
        out.flush();
        com.zeroc.Ice.ProcessPrx process = com.zeroc.Ice.ProcessPrx.checkedCast(processBase);
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
        catch(com.zeroc.Ice.LocalException ex)
        {
            out.println("ok");
        }
        
        test(app._restart == 5);
        test(app._destroyed);
        
        communicator.destroy();
        
        return status;
    }

    public static void main(String[] args)
    {
        Client c = new Client();
        int status = c.main("Client", args);

        System.gc();
        System.exit(status);
    }

    final public PrintWriter out;
}
