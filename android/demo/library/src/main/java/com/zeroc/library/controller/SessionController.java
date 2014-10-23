// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.library.controller;

import android.os.Handler;

public class SessionController
{
    public interface Listener
    {
        void onDestroy();
    }

    private Handler _handler;
    private Ice.Communicator _communicator;
    private QueryController _queryController;
    private Listener _sessionListener;
    private boolean _fatal = false;
    private boolean _destroyed = false;

    private SessionAdapter _session;

    class SessionRefreshThread extends Thread
    {
        SessionRefreshThread(long timeout)
        {
            _timeout = timeout; // seconds.
        }

        synchronized public void
        run()
        {
            while(!_terminated)
            {
                // check idle.
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
                        postSessionDestroyed();
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

        final private long _timeout;
        private boolean _terminated = false;
    }
    private SessionRefreshThread _refresh;

    synchronized private void postSessionDestroyed()
    {
        _fatal = true;
        if(_sessionListener != null)
        {
            final Listener listener = _sessionListener;
            _handler.post(new Runnable()
            {
                public void run()
                {
                    listener.onDestroy();
                }
            });
        }
    }

    SessionController(Handler handler, Ice.Communicator communicator, SessionAdapter session, long refreshTimeout)
    {
        _communicator = communicator;
        _session = session;
        _handler = handler;

        _refresh = new SessionRefreshThread(refreshTimeout);
        _refresh.start();

        _queryController = new QueryController(_handler, _session.getLibrary());
    }

    synchronized public void destroy()
    {
        if(_destroyed)
        {
            return;
        }
        _destroyed = true;

        new Thread(new Runnable()
        {
            public void run()
            {
                _queryController.destroy();

                _refresh.terminate();
                while(_refresh.isAlive())
                {
                    try
                    {
                        _refresh.join();
                    }
                    catch(InterruptedException e)
                    {
                    }
                }

                try
                {
                    _session.destroy();
                }
                catch(Exception e)
                {
                }

                try
                {
                    _communicator.destroy();
                }
                catch(Exception e)
                {
                }
            }
        }).start();
    }

    synchronized public void setSessionListener(Listener listener)
    {
        _sessionListener = listener;
        if(_fatal)
        {
            listener.onDestroy();
        }
    }

    synchronized public QueryController createQuery(QueryController.Listener listener, QueryController.QueryType t,
                                                    String q)
    {
        _queryController.destroy();
        _queryController = new QueryController(_handler, _session.getLibrary(), listener, t, q);
        return _queryController;
    }

    synchronized public QueryController getCurrentQuery()
    {
        return _queryController;
    }
}
