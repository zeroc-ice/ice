// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
            proxy = p;
            session = s;
        }

        Demo.SessionPrx proxy;
        SessionI session;
    }

    synchronized public void
    run()
    {
        while(!_terminated)
        {
            try
            {
                wait(1000);
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
                        if((System.currentTimeMillis() - s.session.timestamp()) > _timeout)
                        {
                            String name = s.proxy.getName();
                            s.proxy.destroy();
                            System.out.println("The session " + name + " has timed out.");
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
        
        _sessions.clear();
    }

    synchronized public void
    add(SessionPrx proxy, SessionI session)
    {
        _sessions.add(new SessionProxyPair(proxy, session));
    }

    private final long _timeout = 10 * 1000; // 10 seconds.
    private boolean _terminated = false;
    private java.util.List<SessionProxyPair> _sessions = new java.util.LinkedList<SessionProxyPair>();
}
