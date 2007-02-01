// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Threading;
using System.Collections;
using Demo;

public class ReapThread
{
    public class SessionProxyPair
    {
        public SessionProxyPair(SessionPrx p, SessionI s)
        {
            proxy = p;
            session = s;
        }

        public SessionPrx proxy;
        public SessionI session;
    }

    public ReapThread()
    {
        _timeout = System.TimeSpan.FromSeconds(10);
        _terminated = false;
        _sessions = new ArrayList();
    }

    public void run()
    {
        lock(this)
        {
            while(!_terminated)
            {
                System.Threading.Monitor.Wait(this, _timeout);
                if(!_terminated)
                {
                    ArrayList tmp = new ArrayList();
                    foreach(SessionProxyPair p in _sessions)
                    {
                        try
                        {
                            //
                            // Session destruction may take time in a
                            // real-world example. Therefore the current time
                            // is computed for each iteration.
                            //
                            if((System.DateTime.Now - p.session.timestamp()) > _timeout)
                            {
                                string name = p.proxy.getName();
                                p.proxy.destroy();
                                Console.Out.WriteLine("The session " + name + " has timed out.");
                            }
                            else
                            {
                                tmp.Add(p);
                            }
                        }
                        catch(Ice.ObjectNotExistException)
                        {
                            // Ignore.
                        }
                    }
                    _sessions = tmp;
                }
            }
        }
    }

    public void terminate()
    {
        lock(this)
        {
            _terminated = true;
            System.Threading.Monitor.Pulse(this);

            _sessions.Clear();
        }
    }

    public void add(SessionPrx proxy, SessionI session)
    {
        lock(this)
        {
            _sessions.Add(new SessionProxyPair(proxy, session));
        }
    }

    private bool _terminated;
    private System.TimeSpan _timeout;
    private ArrayList _sessions;
}
