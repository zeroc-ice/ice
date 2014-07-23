// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
        public long getTimeout();
    }

    static SessionAdapter
    createSession(String appName, Ice.Communicator communicator) {
        SessionAdapter session;
        final Glacier2.RouterPrx router = Glacier2.RouterPrxHelper.uncheckedCast(communicator.getDefaultRouter());
        if(router != null)
        {
            Glacier2.SessionPrx glacier2session = null;
            long timeout;
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
            final long to = timeout;
            final Glacier2SessionPrx sess = Glacier2SessionPrxHelper.uncheckedCast(glacier2session);
            session = new SessionAdapter()
            {
                public LibraryPrx getLibrary()
                {
                    return sess.getLibrary();
                }

                public void destroy()
                {
                    try
                    {
                        router.destroySession();
                    }
                    catch(Glacier2.SessionNotExistException ex)
                    {
                    }
                    catch(Ice.ConnectionLostException ex)
                    {
                        //
                        // Expected: the router closed the connection.
                        //
                    }
                }

                public void refresh()
                {
                    sess.refresh();
                }

                public long getTimeout()
                {
                    return to;
                }
            };
        }
        else
        {
            SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(
                communicator.propertyToProxy("SessionFactory.Proxy"));
            if(factory == null)
            {
                System.err.println(appName + ": invalid object reference");
                return null;
            }

            final SessionPrx sess = factory.create();
            final long timeout = factory.getSessionTimeout()/2;
            session = new SessionAdapter()
            {
                public LibraryPrx getLibrary()
                {
                    return sess.getLibrary();
                }

                public void destroy()
                {
                    sess.destroy();
                }

                public void refresh()
                {
                    sess.refresh();
                }

                 public long getTimeout()
                 {
                    return timeout;
                }
            };
        }
        return session;
    }

    static int
    runParser(String appName, String[] args, final Ice.Communicator communicator)
    {
        final SessionAdapter session = createSession(appName, communicator);
        if(session == null)
        {
            return 1;
        }

        java.util.concurrent.ScheduledExecutorService executor = java.util.concurrent.Executors.newScheduledThreadPool(1);
        executor.scheduleAtFixedRate(new Runnable()
            {
                public void
                run()
                {
                    try
                    {
                        session.refresh();
                    }
                    catch(Ice.LocalException ex)
                    {
                        communicator.getLogger().warning("SessionRefreshThread: " + ex);
                        // Exceptions thrown from the executor task supress subsequent execution
                        // of the task.
                        throw ex;
                    }
                }
            }, session.getTimeout(), session.getTimeout(), java.util.concurrent.TimeUnit.SECONDS);

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

        executor.shutdown();
        session.destroy();

        return rc;
    }
}
