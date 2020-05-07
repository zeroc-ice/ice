//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using Ice;

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
        /// <param name="properties">Optional properties used for communicator initialization.</param>
        /// <param name="logger">Optional logger used for communicator initialization.</param>
        /// <param name="observer">Optional communicator observer used for communicator initialization.</param>
        /// <param name="threadStart">Optional thread start delegate used for communicator initialization.</param>
        /// <param name="threadStop">Optional thread stop delegate used for communicator initialization.</param>
        /// <param name="typeIdNamespaces">Optional list of TypeId namespaces used for communicator initialization.
        /// The default is Ice.TypeId.</param>
        /// <param name="finderStr">The stringified Ice.RouterFinder proxy.</param>
        /// <param name="useCallbacks">True if the session should create an object adapter for receiving callbacks.</param>
        internal SessionHelper(SessionCallback callback,
            string finderStr,
            bool useCallbacks,
            Dictionary<string, string> properties,
            ILogger? logger = null,
            Ice.Instrumentation.ICommunicatorObserver? observer = null,
            Action? threadStart = null,
            Action? threadStop = null,
            string[]? typeIdNamespaces = null)
        {
            _callback = callback;
            _finderStr = finderStr;
            _useCallbacks = useCallbacks;
            _properties = properties;
            _logger = logger;
            _observer = observer;
            _threadStart = threadStart;
            _threadStop = threadStop;
            _typeIdNamespaces = typeIdNamespaces;
        }

        /// <summary>
        /// Destroys the Glacier2 session.
        ///
        /// Once the session has been destroyed, SessionCallback.disconnected is
        /// called on the associated callback object.
        /// </summary>
        public void
        Destroy()
        {
            lock (_mutex)
            {
                if (_destroy)
                {
                    return;
                }
                _destroy = true;
                if (!_connected)
                {
                    //
                    // In this case a connecting session is being destroyed.
                    // We destroy the communicator to trigger the immediate
                    // failure of the connection establishment.
                    //
                    var t1 = new Thread(new ThreadStart(DestroyCommunicator));
                    t1.Start();
                    return;
                }
                _session = null;
                _connected = false;

                //
                // Run destroyInternal in a thread because it makes remote invocations.
                //
                var t2 = new Thread(new ThreadStart(DestroyInternal));
                t2.Start();
            }
        }

        /// <summary>
        /// Returns the session's communicator object.
        /// </summary>
        /// <returns>The communicator.</returns>
        public Communicator?
        Communicator()
        {
            lock (_mutex)
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
        CategoryForClient()
        {
            lock (_mutex)
            {
                if (_router == null)
                {
                    throw new SessionNotExistException();
                }
                Debug.Assert(_category != null);
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
        public IObjectPrx
        AddWithUUID(IObject servant)
        {
            lock (_mutex)
            {
                if (_router == null)
                {
                    throw new SessionNotExistException();
                }
                Debug.Assert(_category != null);
                return InternalObjectAdapter().Add(
                    new Ice.Identity(Guid.NewGuid().ToString(), _category), servant, IObjectPrx.Factory);
            }
        }

        /// <summary>
        /// Returns the Glacier2 session proxy, or null if the session hasn't been
        /// established yet or the session has already been destroyed.
        /// </summary>
        /// <returns>The session proxy, or null if no session exists.</returns>
        public ISessionPrx?
        Session()
        {
            lock (_mutex)
            {
                return _session;
            }
        }

        /// <summary>
        /// Returns true if there is an active session, otherwise returns false.
        /// </summary>
        /// <returns>true if session exists or false if no session exists.</returns>
        public bool
        IsConnected()
        {
            lock (_mutex)
            {
                return _connected;
            }
        }

        /// <summary>
        /// Returns an object adapter for callback objects, creating it if necessary.
        /// </summary>
        /// <return>The object adapter. Throws SessionNotExistException
        /// if no session exists.</return>
        public ObjectAdapter
        ObjectAdapter() => InternalObjectAdapter();

        private ObjectAdapter
        InternalObjectAdapter()
        {
            lock (_mutex)
            {
                if (_router == null)
                {
                    throw new SessionNotExistException();
                }
                if (!_useCallbacks)
                {
                    throw new InvalidOperationException(
                        "Object adapter not available, call SessionFactoryHelper.setUseCallbacks(true)");
                }
                Debug.Assert(_adapter != null);
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
        internal void
        Connect(IReadOnlyDictionary<string, string>? context)
        {
            lock (_mutex)
            {
                ConnectImpl((IRouterPrx router) => router.CreateSessionFromSecureConnection(context)!);
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
        internal void
        Connect(string username, string password, IReadOnlyDictionary<string, string>? context)
        {
            lock (_mutex)
            {
                ConnectImpl((IRouterPrx router) => router.CreateSession(username, password, context)!);
            }
        }

        private void
        Connected(IRouterPrx router, ISessionPrx session)
        {
            //
            // Remote invocation should be done without acquiring a mutex lock.
            //
            Debug.Assert(router != null);
            Debug.Assert(_communicator != null);
            Connection? conn = router.GetCachedConnection();
            string category = router.GetCategoryForClient();
            int acmTimeout = 0;
            try
            {
                acmTimeout = router.GetACMTimeout();
            }
            catch (OperationNotExistException)
            {
            }

            if (acmTimeout <= 0)
            {
                acmTimeout = (int)router.GetSessionTimeout();
            }

            //
            // We create the callback object adapter here because createObjectAdapter internally
            // makes synchronous RPCs to the router. We can't create the OA on-demand when the
            // client calls objectAdapter() or addWithUUID() because they can be called from the
            // GUI thread.
            //
            if (_useCallbacks)
            {
                Debug.Assert(_adapter == null);
                _adapter = _communicator.CreateObjectAdapterWithRouter(router);
                _adapter.Activate();
            }

            lock (_mutex)
            {
                _router = router;

                if (_destroy)
                {
                    //
                    // Run destroyInternal in a thread because it makes remote invocations.
                    //
                    var t = new Thread(new ThreadStart(DestroyInternal));
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

                if (acmTimeout > 0)
                {
                    Connection? connection = _router.GetCachedConnection();
                    Debug.Assert(connection != null);
                    connection.SetACM(acmTimeout, null, ACMHeartbeat.HeartbeatAlways);
                    connection.SetCloseCallback(_ => Destroy());
                }
            }

            try
            {
                _callback.connected(this);
            }
            catch (SessionNotExistException)
            {
                Destroy();
            }
        }

        private void
        DestroyInternal()
        {
            IRouterPrx router;
            Communicator? communicator;
            lock (_mutex)
            {
                Debug.Assert(_destroy);
                if (_router == null)
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
                router.DestroySession();
            }
            catch (ConnectionLostException)
            {
                //
                // Expected if another thread invoked on an object from the session concurrently.
                //
            }
            catch (SessionNotExistException)
            {
                //
                // This can also occur.
                //
            }
            catch (System.Exception e)
            {
                //
                // Not expected.
                //
                communicator.Logger.Warning("SessionHelper: unexpected exception when destroying the session:\n" + e);
            }

            communicator.Destroy();

            // Notify the callback that the session is gone.
            _callback.disconnected(this);
        }

        private void
        DestroyCommunicator()
        {
            Communicator? communicator;
            lock (_mutex)
            {
                communicator = _communicator;
            }
            Debug.Assert(communicator != null);
            communicator.Destroy();
        }

        private delegate ISessionPrx ConnectStrategy(IRouterPrx router);

        private void
        ConnectImpl(ConnectStrategy factory)
        {
            Debug.Assert(!_destroy);
            new Thread(new ThreadStart(() =>
            {
                try
                {
                    lock (_mutex)
                    {
                        _communicator = new Communicator(
                            properties: _properties,
                            logger: _logger,
                            observer: _observer,
                            threadStart: _threadStart,
                            threadStop: _threadStop,
                            typeIdNamespaces: _typeIdNamespaces);
                    }
                }
                catch (System.Exception ex)
                {
                    lock (_mutex)
                    {
                        _destroy = true;
                    }
                    _callback.connectFailed(this, ex);
                    return;
                }

                if (_communicator.DefaultRouter == null)
                {
                    IRouterFinderPrx? finder = null;
                    try
                    {
                        finder = IRouterFinderPrx.Parse(_finderStr, _communicator);
                        _communicator.DefaultRouter = finder.GetRouter();
                    }
                    catch (CommunicatorDestroyedException ex)
                    {
                        _callback.connectFailed(this, ex);
                        return;
                    }
                    catch (System.Exception ex)
                    {
                        if (finder == null)
                        {
                            _callback.connectFailed(this, ex);
                            return;
                        }
                        else
                        {
                            //
                            // In case of error getting router identity from RouterFinder use default identity.
                            //
                            _communicator.DefaultRouter =
                                finder.Clone(new Identity("router", "Glacier2"), Ice.IRouterPrx.Factory);
                        }
                    }
                }

                try
                {
                    _callback.createdCommunicator(this);
                    Ice.IRouterPrx? defaultRouter = _communicator.DefaultRouter;
                    Debug.Assert(defaultRouter != null);
                    var routerPrx = IRouterPrx.UncheckedCast(defaultRouter);
                    ISessionPrx session = factory(routerPrx);
                    Connected(routerPrx, session);
                }
                catch (System.Exception ex)
                {
                    _communicator.Destroy();
                    _callback.connectFailed(this, ex);
                }
            })).Start();
        }

        private Communicator? _communicator;
        private ObjectAdapter? _adapter;
        private IRouterPrx? _router;
        private ISessionPrx? _session;
        private bool _connected = false;
        private string? _category;
        private readonly string _finderStr;
        private readonly bool _useCallbacks;
        private readonly Dictionary<string, string> _properties;
        private readonly Ice.ILogger? _logger;
        private readonly Ice.Instrumentation.ICommunicatorObserver? _observer;
        private readonly Action? _threadStart;
        private readonly Action? _threadStop;
        private readonly string[]? _typeIdNamespaces;

        private readonly SessionCallback _callback;
        private bool _destroy = false;
        private readonly object _mutex = new object();
    }
}
