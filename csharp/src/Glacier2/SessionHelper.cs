// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
    /// <summary>
    /// Creates a Glacier2 session.
    /// </summary>
    /// <param name="callback">The callback for notifications about session
    /// establishment.</param>
    /// <param name="initData">The Ice.InitializationData for initializing
    /// the communicator.</param>
    /// <param name="finderStr">The stringified Ice.RouterFinder proxy.</param>
    /// <param name="useCallbacks">True if the session should create an object adapter for receiving callbacks.</param>
    public SessionHelper(SessionCallback callback, Ice.InitializationData initData, string finderStr, bool useCallbacks)
    {
        _callback = callback;
        _initData = initData;
        _finderStr = finderStr;
        _useCallbacks = useCallbacks;
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
                // In this case a connecting session is being destroyed.
                // We destroy the communicator to trigger the immediate
                // failure of the connection establishment.
                //
                Thread t1 = new Thread(new ThreadStart(destroyCommunicator));
                t1.Start();
                return;
            }
            _session = null;
            _connected = false;

            //
            // Run destroyInternal in a thread because it makes remote invocations.
            //
            Thread t2 = new Thread(new ThreadStart(destroyInternal));
            t2.Start();
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
    /// <return>The object adapter. Throws SessionNotExistException
    /// if no session exists.</return>
    public Ice.ObjectAdapter
    objectAdapter()
    {
        return internalObjectAdapter();
    }

    private Ice.ObjectAdapter
    internalObjectAdapter()
    {
        lock(this)
        {
            if(_router == null)
            {
                throw new SessionNotExistException();
            }
            if(!_useCallbacks)
            {
                throw new Ice.InitializationException(
                    "Object adapter not available, call SessionFactoryHelper.setUseCallbacks(true)");
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
            connectImpl((RouterPrx router) =>
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
            connectImpl((RouterPrx router) =>
                {
                    return router.createSession(username, password, context);
                });
        }
    }

    private void
    connected(RouterPrx router, SessionPrx session)
    {
        //
        // Remote invocation should be done without acquiring a mutex lock.
        //
        Debug.Assert(router != null);
        Ice.Connection conn = router.ice_getCachedConnection();
        string category = router.getCategoryForClient();
        int acmTimeout = 0;
        try
        {
            acmTimeout = router.getACMTimeout();
        }
        catch(Ice.OperationNotExistException)
        {
        }

        if(acmTimeout <= 0)
        {
            acmTimeout = (int)router.getSessionTimeout();
        }

        //
        // We create the callback object adapter here because createObjectAdapter internally
        // makes synchronous RPCs to the router. We can't create the OA on-demand when the
        // client calls objectAdapter() or addWithUUID() because they can be called from the
        // GUI thread.
        //
        if(_useCallbacks)
        {
            Debug.Assert(_adapter == null);
            _adapter = _communicator.createObjectAdapterWithRouter("", router);
            _adapter.activate();
        }

        lock(this)
        {
            _router = router;

            if(_destroy)
            {
                //
                // Run destroyInternal in a thread because it makes remote invocations.
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

            if(acmTimeout > 0)
            {
                Ice.Connection connection = _router.ice_getCachedConnection();
                Debug.Assert(connection != null);
                connection.setACM(acmTimeout, Ice.Util.None, Ice.ACMHeartbeat.HeartbeatAlways);
                connection.setCloseCallback(_ => destroy());
            }
        }

        dispatchCallback(() =>
            {
                try
                {
                    _callback.connected(this);
                }
                catch(SessionNotExistException)
                {
                    destroy();
                }
            }, conn);
    }

    private void
    destroyInternal()
    {
        RouterPrx router;
        Ice.Communicator communicator;
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

        communicator.destroy();

        // Notify the callback that the session is gone.
        dispatchCallback(() =>
            {
                _callback.disconnected(this);
            }, null);
    }

    private void
    destroyCommunicator()
    {
        Ice.Communicator communicator;
        lock(this)
        {
            communicator = _communicator;
        }

        communicator.destroy();
    }

    delegate SessionPrx ConnectStrategy(RouterPrx router);

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
                new ThreadStart(() =>
                    {
                        dispatchCallback(() =>
                            {
                                _callback.connectFailed(this, ex);
                            },
                            null);
                    })).Start();
            return;
        }

        Ice.RouterFinderPrx finder = Ice.RouterFinderPrxHelper.uncheckedCast(_communicator.stringToProxy(_finderStr));
        new Thread(new ThreadStart(() =>
        {
            if(_communicator.getDefaultRouter() == null)
            {
                try
                {
                    _communicator.setDefaultRouter(finder.getRouter());
                }
                catch(Ice.CommunicatorDestroyedException ex)
                {
                    dispatchCallback(() =>
                    {
                        _callback.connectFailed(this, ex);
                    }, null);
                    return;
                }
                catch(Exception)
                {
                    //
                    // In case of error getting router identity from RouterFinder use default identity.
                    //
                    Ice.Identity ident = new Ice.Identity("router", "Glacier2");
                    _communicator.setDefaultRouter(Ice.RouterPrxHelper.uncheckedCast(finder.ice_identity(ident)));
                }
            }

            try
            {
                dispatchCallbackAndWait(() =>
                    {
                        _callback.createdCommunicator(this);
                    });

                RouterPrx routerPrx = RouterPrxHelper.uncheckedCast(_communicator.getDefaultRouter());
                SessionPrx session = factory(routerPrx);
                connected(routerPrx, session);
            }
            catch(Exception ex)
            {
                _communicator.destroy();

                dispatchCallback(() =>
                    {
                        _callback.connectFailed(this, ex);
                    }, null);
            }
        })).Start();
    }

    private void
    dispatchCallback(Action callback, Ice.Connection conn)
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

    private void
    dispatchCallbackAndWait(Action callback)
    {
        if(_initData.dispatcher != null)
        {
            EventWaitHandle h = new ManualResetEvent(false);
            _initData.dispatcher(() =>
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
    private RouterPrx _router;
    private SessionPrx _session;
    private bool _connected = false;
    private string _category;
    private string _finderStr;
    private bool _useCallbacks;

    private readonly SessionCallback _callback;
    private bool _destroy = false;
}

}
