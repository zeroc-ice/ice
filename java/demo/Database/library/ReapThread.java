// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class ReapThread extends Thread
{
    static class SessionProxyPair
    {
        SessionProxyPair(Demo.SessionPrx p, SessionI s)
        {
            glacier2proxy = null;
            proxy = p;
            session = s;
        }

        SessionProxyPair(Glacier2.SessionPrx p, SessionI s)
        {
            glacier2proxy = p;
            proxy = null;
            session = s;
        }

        Glacier2.SessionPrx glacier2proxy;
        Demo.SessionPrx proxy;
        SessionI session;
    }

    ReapThread(Ice.Logger logger, long timeout)
    {
        _logger = logger;
        _timeout = timeout;
    }

    synchronized public void
    run()
    {
        while(!_terminated)
        {
            try
            {
                wait((_timeout / 2) * 1000);
            }
            catch(InterruptedException e)
            {
            }

            if(!_terminated)
            {
                java.util.Iterator<SessionProxyPair> p = _sessions.iterator();
                while(p.hasNext())
                {
                    SessionProxyPair s = p.next();
                    try
                    {
                        //
                        // Session destruction may take time in a
                        // real-world example. Therefore the current time
                        // is computed for each iteration.
                        //
                        if((System.currentTimeMillis() - s.session.timestamp()) > _timeout * 1000)
                        {
                            _logger.trace("ReapThread", "The session " +
                                          s.proxy.ice_getCommunicator().identityToString(s.proxy.ice_getIdentity()) +
                                          " has timed out.");
                            if(s.proxy != null)
                            {
                                s.proxy.destroy();
                            }
                            else
                            {
                                s.glacier2proxy.destroy();
                            }
                            p.remove();
                        }
                    }
                    catch(Ice.ObjectNotExistException e)
                    {
                        p.remove();
                    }
                }
            }
        }
    }

    synchronized public void
    terminate()
    {
        _terminated = true;
        notify();

        // Destroy each of the sessions, releasing any resources they
        // may hold. This calls directly on the session, not via the
        // proxy since terminate() is called after the communicator is
        // shutdown, which means calls on collocated objects are not
        // permitted.
        for(SessionProxyPair p : _sessions)
        {
            p.session.shutdown();
        }
        _sessions.clear();
    }

    synchronized public void
    add(SessionPrx proxy, SessionI session)
    {
        _sessions.add(new SessionProxyPair(proxy, session));
    }

    synchronized public void
    add(Glacier2.SessionPrx proxy, SessionI session)
    {
        _sessions.add(new SessionProxyPair(proxy, session));
    }

    private final long _timeout; // Seconds.
    private Ice.Logger _logger;
    private boolean _terminated = false;
    private java.util.List<SessionProxyPair> _sessions = new java.util.LinkedList<SessionProxyPair>();
}
