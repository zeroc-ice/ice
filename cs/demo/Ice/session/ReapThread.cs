// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
#if COMPACT
        _m.Lock();
        try
        {
            while(!_terminated)
            {
                _m.TimedWait(1000);
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
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            while(!_terminated)
            {
                System.Threading.Monitor.Wait(this, System.TimeSpan.FromSeconds(1));
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
#endif
    }

    public void terminate()
    {
#if COMPACT
        _m.Lock();
        try
        {
            _terminated = true;
            _m.Notify();

            _sessions.Clear();
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            _terminated = true;
            System.Threading.Monitor.Pulse(this);

            _sessions.Clear();
        }
#endif
    }

    public void add(SessionPrx proxy, SessionI session)
    {
#if COMPACT
        _m.Lock();
        try
        {
            _sessions.Add(new SessionProxyPair(proxy, session));
        }
        finally
        {
            _m.Unlock();
        }
#else
        lock(this)
        {
            _sessions.Add(new SessionProxyPair(proxy, session));
        }
#endif
    }

    private bool _terminated;
    private System.TimeSpan _timeout;
    private ArrayList _sessions;
#if COMPACT
    private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
#endif
}
