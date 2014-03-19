// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading;
using System.Collections.Generic;

namespace Glacier2
{

/// <summary>
/// A helper class for using Glacier2 with GUI applications.
/// </summary>
public class SessionHelper
{
    private class SessionRefreshThread
    {
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
            _m.Lock();
            try
            {
                while(true)
                {
                    try
                    {
                        _router.begin_refreshSession().whenCompleted(
                                            (Ice.Exception ex) => 
                                                        {
                                                            this.done();
                                                            _session.destroy();
                                                        });
                    }
                    catch(Ice.CommunicatorDestroyedException)
                    {
                        //
                        // AMI requests can raise CommunicatorDestroyedException directly.
                        //
                        break;
                    }

                    if(!_done)
                    {
#if COMPACT || SILVERLIGHT
                        _m.TimedWait(_period);
#else
                        try
                        {
                            _m.TimedWait(_period);
                        }
                        catch(ThreadInterruptedException)
                        {
                        }
#endif
                    }

                    if(_done)
                    {
                        break;
                    }
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        public void
        done()
        {
            _m.Lock();
            try
            {
                if(!_done)
                {
                    _done = true;
                    _m.Notify();
                }
            }
            finally
            {
                _m.Unlock();
            }
        }

        private SessionHelper _session;
        private Glacier2.RouterPrx _router;
        private int _period;
        private bool _done = false;

        private readonly IceUtilInternal.Monitor _m = new IceUtilInternal.Monitor();
    }

    /// <summary>
    /// Creates a Glacier2 session.
    /// </summary>
    /// <param name="callback">The callback for notifications about session
    /// establishment.</param>
    /// <param name="initData">The Ice.InitializationData for initializing
    /// the communicator.</param>
    public SessionHelper(SessionCallback callback, Ice.InitializationData initData)
    {
        _callback = callback;
        _initData = initData;
    }

    /// <summary>
    /// Destroys the Glacier2 session.
    ///
    /// Once the session has been destroyed, SessionCallback.disconnected is
    /// called on the associated callback object.
    /// </summary>
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
            if(!_connected)
            {
                //
                // In this case a connecting session is being
                // destroyed. The communicator and session will be
                // destroyed when the connection establishment has
                // completed.
                //
                return;
            }
            _session = null;
            _connected = false;
            //
            // Run the destroyInternal in a thread. This is because it
            // destroyInternal makes remote invocations.
            //
            Thread t = new Thread(new ThreadStart(destroyInternal));
            t.Start();
        }
    }

    /// <summary>
    /// Returns the session's communicator object.
    /// </summary>
    /// <returns>The communicator.</returns>
    public Ice.Communicator
    communicator()
    {
        lock(this)
        {
            return _communicator;
        }
    }

    /// <summary>
    /// Returns the category to be used in the identities of all of
    /// the client's callback objects. Clients must use this category
    /// for the router to forward callback requests to the intended
    /// client.
    /// </summary>
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

            return _category;
        }
    }

    /// <summary>
    /// Adds a servant to the callback object adapter's Active Servant
    /// Map with a UUID.
    /// </summary>
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

            return internalObjectAdapter().add(servant, new Ice.Identity(Guid.NewGuid().ToString(), _category));
        }
    }

    /// <summary>
    /// Returns the Glacier2 session proxy. If the session hasn't been
    /// established yet, or the session has already been destroyed,
    /// throws SessionNotExistException.
    /// </summary>
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

    /// <summary>
    /// Returns true if there is an active session, otherwise returns false.
    /// </summary>
    /// <returns>true if session exists or false if no session exists.</returns>
    public bool
    isConnected()
    {
        lock(this)
        {
            return _connected;
        }
    }

    /// <summary>
    /// Returns an object adapter for callback objects, creating it if necessary.
    /// </summary>
    /// <return>The object adapter. throws SessionNotExistException
    /// if no session exists.</return>
    public Ice.ObjectAdapter
    objectAdapter()
    {
        return internalObjectAdapter();
    }

    //
    // Only call this method when the calling thread owns the lock
    //
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

    /// <summary>
    /// Connects to the Glacier2 router using the associated SSL credentials.
    ///
    /// Once the connection is established, SessionCallback.connected is called on
    /// the callback object; upon failure, SessionCallback.exception is called with
    /// the exception.
    /// </summary>
    /// <param name="context">The request context to use when creating the session.</param>
    public void
    connect(Dictionary<string, string> context)
    {
        lock(this)
        {
            connectImpl(delegate(RouterPrx router)
                            {
                                return router.createSessionFromSecureConnection(context);
                            });
        }
    }

    /// <summary>
    /// Connects a Glacier2 session using user name and password credentials.
    ///
    /// Once the connection is established, SessionCallback.connected is called on the callback object;
    /// upon failure SessionCallback.exception is called with the exception.
    /// </summary>
    /// <param name="username">The user name.</param>
    /// <param name="password">The password.</param>
    /// <param name="context">The request context to use when creating the session.</param>
    public void
    connect(string username, string password, Dictionary<string, string> context)
    {
        lock(this)
        {
            connectImpl(delegate(RouterPrx router)
                            {
                                return router.createSession(username, password, context);
                            });
        }
    }

    private void
    connected(RouterPrx router, SessionPrx session)
    {
        string category = router.getCategoryForClient();
        long timeout = router.getSessionTimeout();
        Ice.Connection conn = router.ice_getCachedConnection();
        lock(this)
        {
            _router = router;

            if(_destroy)
            {
                //
                // Run the destroyInternal in a thread. This is because it
                // destroyInternal makes remote invocations.
                //
                Thread t = new Thread(new ThreadStart(destroyInternal));
                t.Start();
                return;
            }

            //
            // Cache the category.
            //
            _category = category;

            //
            // Assign the session after _destroy is checked.
            //
            _session = session;
            _connected = true;

            Debug.Assert(_sessionRefresh == null);
            if(timeout > 0)
            {
                _sessionRefresh = new SessionRefreshThread(this, _router, (int)(timeout * 1000)/2);
                _refreshThread = new Thread(new ThreadStart(_sessionRefresh.run));
                _refreshThread.Start();
            }
        }

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
                         }, conn);
    }

    private void
    destroyInternal()
    {
        Glacier2.RouterPrx router;
        Ice.Communicator communicator;
        SessionRefreshThread sessionRefresh;
        lock(this)
        {
            Debug.Assert(_destroy);
            if(_router == null)
            {
                return;
            }
            router = _router;
            _router = null;

            communicator = _communicator;

            Debug.Assert(communicator != null);

            sessionRefresh = _sessionRefresh;
            _sessionRefresh = null;
        }

        try
        {
            router.destroySession();
        }
        catch(Ice.ConnectionLostException)
        {
            //
            // Expected if another thread invoked on an object from the session concurrently.
            //
        }
        catch(SessionNotExistException)
        {
            //
            // This can also occur.
            //
        }
        catch(Exception e)
        {
            //
            // Not expected.
            //
            communicator.getLogger().warning("SessionHelper: unexpected exception when destroying the session:\n" + e);
        }

        if(sessionRefresh != null)
        {
            sessionRefresh.done();
            while(true)
            {
#if COMPACT || SILVERLIGHT
                _refreshThread.Join();
                break;
#else
                try
                {
                    _refreshThread.Join();
                    break;
                }
                catch(ThreadInterruptedException)
                {
                }
#endif
            }
            _refreshThread = null;
        }


        try
        {
            communicator.destroy();
        }
        catch(Exception)
        {
        }

        // Notify the callback that the session is gone.
        dispatchCallback(delegate()
                         {
                             _callback.disconnected(this);
                         }, null);
    }

    delegate Glacier2.SessionPrx ConnectStrategy(Glacier2.RouterPrx router);

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
            new Thread(
                new ThreadStart(delegate()
                    {
                        dispatchCallback(delegate()
                            {
                                _callback.connectFailed(this, ex);
                            }, 
                            null);
                    })).Start();
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
                                     }, null);
                }
        })).Start();
    }

#if COMPACT
    private void
    dispatchCallback(Ice.VoidAction callback, Ice.Connection conn)
#else
    private void
    dispatchCallback(System.Action callback, Ice.Connection conn)
#endif
    {
        if(_initData.dispatcher != null)
        {
            _initData.dispatcher(callback, conn);
        }
        else
        {
            callback();
        }
    }

#if COMPACT
    private void
    dispatchCallbackAndWait(Ice.VoidAction callback)
#else
    private void
    dispatchCallbackAndWait(System.Action callback)
#endif
    {
        if(_initData.dispatcher != null)
        {
            EventWaitHandle h = new ManualResetEvent(false);
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

    private readonly Ice.InitializationData _initData;
    private Ice.Communicator _communicator;
    private Ice.ObjectAdapter _adapter;
    private Glacier2.RouterPrx _router;
    private Glacier2.SessionPrx _session;
    private bool _connected = false;
    private string _category;

    private SessionRefreshThread _sessionRefresh;
    private Thread _refreshThread;
    private readonly SessionCallback _callback;
    private bool _destroy = false;
}

}
