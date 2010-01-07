// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;

namespace Glacier2
{

/// <sumary>
/// A helper class for using Glacier2 with GUI applications.
/// </sumary>
public class SessionHelper
{
    private class SessionRefreshThread
    {
        private class RefreshI : Glacier2.AMI_Router_refreshSession
        {
            public
            RefreshI(SessionHelper helper, SessionRefreshThread thread)
            {
                _thread = thread;
                _helper = helper;
            }

            public override void ice_response()
            {
            }

            public override void ice_exception(Ice.Exception ex)
            {
                _thread.done();
                _helper.destroy();
            }

            SessionRefreshThread _thread;
            SessionHelper _helper;
        }

        public SessionRefreshThread(SessionHelper session, Glacier2.RouterPrx router, int period)
        {
            _session = session;
            _router = router;
            _period = period;
            _done = false;
        }

        public void
        run()
        {
            lock(this)
            {
                while(true)
                {
                    _router.refreshSession_async(new RefreshI(_session, this));
                    if(!_done)
                    {
                        try
                        {
                            System.Threading.Monitor.Wait(this, _period);
                        }
                        catch(ThreadInterruptedException)
                        {
                        }
                    }
                    if(_done)
                    {
                        break;
                    }
                }
            }
        }

        public void
        done()
        {
            lock(this)
            {
                if(!_done)
                {
                    _done = true;
                    System.Threading.Monitor.Pulse(this);
                }
            }
        }

        private SessionHelper _session;
        private Glacier2.RouterPrx _router;
        private int _period;
        private bool _done = false;
    }

    /// <sumary>
    /// Creates a Glacier2 session.
    /// </sumary>
    /// <param name="callback">The callback for notifications about session
    /// establishment.</param>
    /// <param name="initData">The Ice.InitializationData for initializing
    /// the communicator.</param>
    public SessionHelper(SessionCallback callback, Ice.InitializationData initData)
    {
        _callback = callback;
        _initData = initData;
    }

    /// <sumary>
    /// Destroys the Glacier2 session.
    /// 
    /// Once the session has been destroyed, SessionCallback.disconnected is
    /// called on the associated callback object.
    /// </sumary>
    public void
    destroy()
    {
        lock(this)
        {
            if(_destroy)
            {
                return;
            }
            _destroy = true;

            if(_sessionRefresh == null)
            {
                // In this case a connecting session is being
                // destroyed. The communicator and session will be
                // destroyed when the connection establishment has
                // completed.
                return;
            }
            _session = null;

            // Run the destroyInternal in a thread. This is because it
            // destroyInternal makes remote invocations.
            Thread t = new Thread(new ThreadStart(destroyInternal));
            t.Start();
        }
    }

    /// <sumary>
    /// Returns the session's communicator object.
    /// </sumary>
    /// <returns>The communicator.</returns>
    public Ice.Communicator
    communicator()
    {
        lock(this)
        {
            return _communicator;
        }
    }

    /// <sumary>
    /// Returns the category to be used in the identities of all of 
    /// the client's callback objects. Clients must use this category 
    /// for the router to forward callback requests to the intended
    /// client.
    /// </sumary>
    /// <returns>The category. Throws SessionNotExistException 
    /// No session exists</returns>
    public string
    categoryForClient()
    {
        lock(this)
        {
            if(_router == null)
            {
                throw new SessionNotExistException();
            }
            
            return _router.getCategoryForClient();
        }
    }

    /// <sumary>
    /// Adds a servant to the callback object adapter's Active Servant 
    /// Map with a UUID.
    /// </sumary>
    /// <param name="servant">The servant to add.</param>
    /// <returns>The proxy for the servant. Throws SessionNotExistException 
    /// if no session exists.</returns>
    public Ice.ObjectPrx
    addWithUUID(Ice.Object servant)
    {
        lock(this)
        {
            if(_router == null)
            {
                throw new SessionNotExistException();
            }
            
            return internalObjectAdapter().add(servant, new Ice.Identity(Guid.NewGuid().ToString(),
                                                                         _router.getCategoryForClient()));
        }
    }
    
    /// <sumary>
    /// Returns the Glacier2 session proxy. If the session hasn't been 
    /// established yet, or the session has already been destroyed, 
    /// throws SessionNotExistException.
    /// </sumary>
    /// <returns>The session proxy, or throws SessionNotExistException  
    /// if no session exists.</returns>
    public Glacier2.SessionPrx
    session()
    {
        lock(this)
        {
            if(_session == null)
            {
                throw new SessionNotExistException();
            }
            return _session;
        }
    }

    /// <sumary>
    /// Returns true if there is an active session, otherwise returns false.
    /// </sumary>
    /// <returns>true if session exists or false if no session exists.</returns>
    public bool
    isConnected()
    {
        lock(this)
        {
            return _connected;
        }
    }

    /// <sumary>
    /// Creates an object adapter for callback objects.
    /// </sumary>
    /// <return>The object adapter. throws SessionNotExistException 
    /// if no session exists.</return>
    public Ice.ObjectAdapter
    objectAdapter()
    {
        return internalObjectAdapter();
    }

    // Only call this method when the calling thread owns the lock
    private Ice.ObjectAdapter
    internalObjectAdapter()
    {
        lock(this)
        {
            if(_router == null)
            {
                throw new SessionNotExistException();
            }
            if(_adapter == null)
            {
                _adapter = _communicator.createObjectAdapterWithRouter("", _router);
                _adapter.activate();
            }
            return _adapter;
        }
    }
    
    /// <sumary>
    /// Connects to the Glacier2 router using the associated SSL credentials.
    ///
    /// Once the connection is established, SessionCallback.connected} is called on 
    /// the callback object; upon failure, SessionCallback.exception is called with
    /// the exception.
    /// </sumary>
    public void
    connect()
    {
        lock(this)
        {
            connectImpl(delegate(RouterPrx router)
                            {
                                return router.createSessionFromSecureConnection();
                            });
        }
    }

    /// <sumary>
    /// Connects a Glacier2 session using user name and password credentials.
    ///
    /// Once the connection is established, SessionCallback.connected is called on the callback object;
    /// upon failure SessionCallback.exception is called with the exception. 
    /// </sumary>
    /// <param name="username">The user name.</param>
    /// <param name="password">The password.</param>
    public void
    connect(string username, string password)
    {
        lock(this)
        {
            connectImpl(delegate(RouterPrx router)
                            {
                                return router.createSession(username, password);
                            });
        }
    }

    private void
    connected(RouterPrx router, SessionPrx session)
    {
        lock(this)
        {
            _router = router;

            if(_destroy)
            {
                destroyInternal();
                return;
            }

            // Assign the session after _destroy is checked.
            _session = session;
            _connected = true;

            Debug.Assert(_sessionRefresh == null);
            _sessionRefresh = new SessionRefreshThread(this, _router, (int)(_router.getSessionTimeout() * 1000)/2);
            _refreshThread = new Thread(new ThreadStart(_sessionRefresh.run));
            _refreshThread.Start();


            dispatchCallback(delegate()
                             {
                                 try
                                 {
                                     _callback.connected(this);
                                 }
                                 catch(Glacier2.SessionNotExistException)
                                 {
                                     destroy();
                                 }
                             });
        }
    }

    private void
    destroyInternal()
    {
        lock(this)
        {
            Debug.Assert(_destroy);

            try
            {
                _router.destroySession();
            }
            catch(Ice.ConnectionLostException)
            {
                // Expected if another thread invoked on an object from the session concurrently.
            }
            catch(SessionNotExistException)
            {
                // This can also occur.
            }
            catch(Exception e)
            {
                // Not expected.
                _communicator.getLogger().warning("SessionHelper: unexpected exception when destroying the session:\n"
                                                  + e);
            }
            _router = null;
            _connected = false;
            if(_sessionRefresh != null)
            {
                _sessionRefresh.done();
                while(true)
                {
                    try
                    {
                        _refreshThread.Join();
                        break;
                    }
                    catch(ThreadInterruptedException)
                    {
                    }
                }
                _sessionRefresh = null;
                _refreshThread = null;
            }

            try
            {
                _communicator.destroy();
            }
            catch(Exception)
            {
            }
            _communicator = null;

            // Notify the callback that the session is gone.
            dispatchCallback(delegate()
                             {
                                 _callback.disconnected(this);
                             });
        }
    }
    
    delegate Glacier2.SessionPrx
    ConnectStrategy(Glacier2.RouterPrx router);

    private void
    connectImpl(ConnectStrategy factory)
    {
        Debug.Assert(!_destroy);

        try
        {
            _communicator = Ice.Util.initialize(_initData);
        }
        catch(Ice.LocalException ex)
        {
            _destroy = true;
            dispatchCallback(delegate()
                             {
                                 _callback.connectFailed(this, ex);
                             });
            return;
        }

        new Thread(new ThreadStart(delegate()
        {
                try
                {
                    dispatchCallbackAndWait(delegate()
                                            {
                                                _callback.createdCommunicator(this);
                                            });

                    Glacier2.RouterPrx routerPrx = Glacier2.RouterPrxHelper.uncheckedCast(
                        _communicator.getDefaultRouter());
                    Glacier2.SessionPrx session = factory(routerPrx);
                    connected(routerPrx, session);
                }
                catch (Exception ex)
                {
                    try
                    {
                        _communicator.destroy();
                    }
                    catch(Exception)
                    {
                    }

                    dispatchCallback(delegate()
                                     {
                                         _callback.connectFailed(this, ex);
                                     });
                }
        })).Start();
    }

    private void
    dispatchCallback(System.Action callback)
    {
        if(_initData.dispatcher != null)
        {
            _initData.dispatcher(callback, null);
        }
        else
        {
            callback();
        }
    }

    private void
    dispatchCallbackAndWait(System.Action callback)
    {
        if(_initData.dispatcher != null)
        {
            EventWaitHandle h = new EventWaitHandle(false, EventResetMode.ManualReset);
            _initData.dispatcher(delegate()
                                 {
                                     callback();
                                     h.Set();
                                 }, null);
            h.WaitOne();
        }
        else
        {
            callback();
        }
    }

    private Ice.InitializationData _initData;
    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _adapter;
    private Glacier2.RouterPrx _router;
    private Glacier2.SessionPrx _session;
    private bool _connected = false;

    private SessionRefreshThread _sessionRefresh;
    private Thread _refreshThread;
    private SessionCallback _callback;
    private bool _destroy = false;
}

}
