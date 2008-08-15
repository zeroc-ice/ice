// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class RunParser
{
    static private class SessionRefreshThread extends Thread
    {
        SessionRefreshThread(Ice.Logger logger, long timeout, SessionPrx session)
        {
            _logger = logger;
            _session = session;
            _timeout = timeout;
        }

        synchronized public void
        run()
        {
            while(!_terminated)
            {
                try
                {
                    wait(_timeout);
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
        final private SessionPrx _session;
        final private long _timeout;
        private boolean _terminated = false;
    }

    static int
    runParser(String appName, String[] args, Ice.Communicator communicator)
    {
        SessionFactoryPrx factory = SessionFactoryPrxHelper.checkedCast(
            communicator.propertyToProxy("SessionFactory.Proxy"));
        if(factory == null)
        {
            System.err.println(appName + ": invalid object reference");
            return 1;
        }

        SessionPrx session = factory.create();
        SessionRefreshThread refresh = new SessionRefreshThread(communicator.getLogger(), 5000, session);
        refresh.start();

        LibraryPrx library = session.getLibrary();

        Parser parser = new Parser(communicator, library);
        int rc = parser.parse();

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
