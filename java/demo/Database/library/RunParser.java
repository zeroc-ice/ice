// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class RunParser
{
    //
    // Adapter for the two types of session objects.
    //
    interface SessionAdapter
    {
        public LibraryPrx getLibrary();
        public void destroy();
        public void refresh();
    }

    static class Glacier2SessionAdapter implements SessionAdapter
    {
        public LibraryPrx getLibrary()
        {
            return _session.getLibrary();
        }

        public void destroy()
        {
            _session.destroy();
        }

        public void refresh()
        {
            _session.refresh();
        }

        Glacier2SessionAdapter(Glacier2SessionPrx session)
        {
            _session = session;
        }

        private Glacier2SessionPrx _session;
    }

    static class DemoSessionAdapter implements SessionAdapter
    {
        public LibraryPrx getLibrary()
        {
            return _session.getLibrary();
        }

        public void destroy()
        {
            _session.destroy();
        }

        public void refresh()
        {
            _session.refresh();
        }

        DemoSessionAdapter(SessionPrx session)
        {
            _session = session;
        }

        private SessionPrx _session;
    }

    static private class SessionRefreshThread extends Thread
    {
        SessionRefreshThread(Ice.Logger logger, long timeout, SessionAdapter session)
        {
            _logger = logger;
            _session = session;
            _timeout = timeout; // seconds.
        }

        synchronized public void
        run()
        {
            while(!_terminated)
            {
                try
                {
                    wait(_timeout * 1000);
                }
                catch(InterruptedException e)
                {
                }
                if(!_terminated)
                {
                    try
                    {
                        _session.refresh();
                    }
                    catch(Ice.LocalException ex)
                    {
                        _logger.warning("SessionRefreshThread: " + ex);
                        _terminated = true;
                    }
                }
            }
        }

        synchronized private void
        terminate()
        {
            _terminated = true;
            notify();
        }

        final private Ice.Logger _logger;
        final private SessionAdapter _session;
        final private long _timeout;
        private boolean _terminated = false;
    }

    static int
    runParser(String appName, String[] args, Ice.Communicator communicator)
    {
        SessionAdapter session;
        Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.uncheckedCast(communicator.getDefaultRouter());
        long timeout;
        if(router != null)
        {
            Glacier2.SessionPrx glacier2session = null;
            java.io.BufferedReader in = new java.io.BufferedReader(new java.io.InputStreamReader(System.in));
            while(true)
            {
                System.out.println("This demo accepts any user-id / password combination.");

                try
                {
                    String id;
                    System.out.print("user id: ");
                    System.out.flush();
                    id = in.readLine();

                    String pw;
                    System.out.print("password: ");
                    System.out.flush();
                    pw = in.readLine();

                    try
                    {
                        glacier2session = router.createSession(id, pw);
                        timeout = router.getSessionTimeout() / 2;
                        break;
                    }
                    catch(Glacier2.PermissionDeniedException ex)
                    {
                        System.out.println("permission denied:\n" + ex.reason);
                    }
                    catch(Glacier2.CannotCreateSessionException ex)
                    {
                        System.out.println("cannot create session:\n" + ex.reason);
                    }
                }
                catch(java.io.IOException ex)
                {
                    ex.printStackTrace();
                }
            }
            session = new Glacier2SessionAdapter(Glacier2SessionPrxHelper.uncheckedCast(glacier2session));
        }
        else
        {
            SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(
                communicator.propertyToProxy("SessionFactory.Proxy"));
            if(factory == null)
            {
                System.err.println(appName + ": invalid object reference");
                return 1;
            }

            session = new DemoSessionAdapter(factory.create());
            timeout = factory.getSessionTimeout()/2;
        }
        SessionRefreshThread refresh = new SessionRefreshThread(communicator.getLogger(), timeout, session);
        refresh.start();

        LibraryPrx library = session.getLibrary();

        Parser parser = new Parser(communicator, library);

        int rc = 0;

        if(args.length == 1)
        {
            rc = parser.parse(args[0]);
        }

        if(rc == 0)
        {
            rc = parser.parse();
        }

        if(refresh != null)
        {
            refresh.terminate();
            try
            {
                refresh.join();
            }
            catch(InterruptedException e)
            {
            }
            refresh = null;
        }

        session.destroy();

        return rc;
    }
}
