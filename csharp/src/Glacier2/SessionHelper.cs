//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Net.Security;
using System.Diagnostics;
using System.Security.Cryptography.X509Certificates;
using System.Threading;
using ZeroC.Ice;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Glacier2
{
    /// <summary>A helper class for using Glacier2 with GUI applications.</summary>
    public class SessionHelper
    {
        /// <summary>Gets the session's communicator object.</summary>
        public Communicator? Communicator
        {
            get
            {
                lock (_mutex)
                {
                    return _communicator;
                }
            }
        }

        /// <summary>Gets whether there is an active session to the Glacier2 router.</summary>
        public bool IsConnected
        {
            get
            {
                lock (_mutex)
                {
                    return _connected;
                }
            }
        }

        /// <summary>Gets the Glacier2 session proxy, or null if the session hasn't been established yet or the session
        /// has already been destroyed.</summary>
        public ISessionPrx? Session
        {
            get
            {
                lock (_mutex)
                {
                    return _session;
                }
            }
        }

        private ObjectAdapter? _adapter;
        private readonly ISessionCallback _callback;
        private readonly X509Certificate2Collection? _caCertificates;
        private string? _category;
        private readonly X509Certificate2Collection? _certificates;
        private readonly RemoteCertificateValidationCallback? _certificateValidationCallback;
        private Communicator? _communicator;
        private bool _connected = false;
        private bool _destroy = false;
        private readonly string _finderStr;
        private readonly ILogger? _logger;
        private readonly object _mutex = new object();
        private readonly ICommunicatorObserver? _observer;
        private readonly IPasswordCallback? _passwordCallback;
        private readonly Dictionary<string, string> _properties;
        private IRouterPrx? _router;
        private ISessionPrx? _session;
        private readonly bool _useCallbacks;

        /// <summary>Adds a servant to the callback object adapter's with an UUID.</summary>
        /// <param name="servant">The servant to add.</param>
        /// <returns>The proxy for the servant. Throws SessionNotExistException if no session exists.</returns>
        public IObjectPrx AddWithUUID(IObject servant)
        {
            lock (_mutex)
            {
                if (_router == null)
                {
                    throw new SessionNotExistException();
                }
                Debug.Assert(_category != null);
                return ObjectAdapter().Add(
                    new Identity(Guid.NewGuid().ToString(), _category), servant, IObjectPrx.Factory);
            }
        }

        /// <summary>Destroys the Glacier2 session. Once the session has been destroyed, SessionCallback.disconnected
        /// is called on the associated callback object.</summary>
        public void Destroy()
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
                    // In this case a connecting session is being destroyed. We destroy the communicator to trigger the
                    // immediate failure of the connection establishment.
                    var t1 = new Thread(new ThreadStart(DestroyCommunicator));
                    t1.Start();
                    return;
                }
                _session = null;
                _connected = false;

                // Run destroyInternal in a thread because it makes remote invocations.
                var t2 = new Thread(new ThreadStart(DestroyInternal));
                t2.Start();
            }
        }

        /// <summary>Returns the category to be used in the identities of all of the client's callback objects. Clients
        /// must use this category for the router to forward callback requests to the intended client.</summary>
        /// <returns>The category. Throws SessionNotExistException if no session exists</returns>
        public string GetCategoryForClient()
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

        /// <summary>Returns an object adapter for callback objects, creating it if necessary.</summary>
        /// <return>The object adapter. Throws SessionNotExistException if no session exists.</return>
        public ObjectAdapter ObjectAdapter()
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

        internal SessionHelper(ISessionCallback callback,
            string finderStr,
            bool useCallbacks,
            Dictionary<string, string> properties,
            ILogger? logger = null,
            ICommunicatorObserver? observer = null,
            X509Certificate2Collection? certificates = null,
            X509Certificate2Collection? caCertificates = null,
            RemoteCertificateValidationCallback? _certificateValidationCallback = null,
            IPasswordCallback? passwordCallback = null)
        {
            _callback = callback;
            _finderStr = finderStr;
            _useCallbacks = useCallbacks;
            _properties = properties;
            _logger = logger;
            _observer = observer;
            _certificates = certificates;
            _caCertificates = caCertificates;
            this._certificateValidationCallback = _certificateValidationCallback;
            _passwordCallback = passwordCallback;
        }

        /// <summary>Connects to the Glacier2 router using the associated SSL credentials. Once the connection is
        /// established, SessionCallback.connected is called on the callback object; upon failure,
        /// SessionCallback.exception is called with the exception.</summary>
        /// <param name="context">The request context to use when creating the session.</param>
        internal void Connect(IReadOnlyDictionary<string, string>? context)
        {
            lock (_mutex)
            {
                ConnectImpl((IRouterPrx router) => router.CreateSessionFromSecureConnection(context)!);
            }
        }

        /// <summary>Connects a Glacier2 session using username and password credentials. Once the connection is
        /// established, SessionCallback.connected is called on the callback object; upon failure
        /// SessionCallback.exception is called with the exception.</summary>
        /// <param name="username">The user name.</param>
        /// <param name="password">The password.</param>
        /// <param name="context">The request context to use when creating the session.</param>
        internal void Connect(string username, string password, IReadOnlyDictionary<string, string>? context)
        {
            lock (_mutex)
            {
                ConnectImpl((IRouterPrx router) => router.CreateSession(username, password, context)!);
            }
        }

        private void Connected(IRouterPrx router, ISessionPrx session)
        {
            // Remote invocation should be done without acquiring a mutex lock.
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

            // We create the callback object adapter here because createObjectAdapter internally makes synchronous
            // RPCs to the router. We can't create the OA on-demand when the client calls ObjectAdapter() or
            // AddWithUUID() because they can be called from the GUI thread.
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
                    // Run destroyInternal in a thread because it makes remote invocations.
                    var t = new Thread(new ThreadStart(DestroyInternal));
                    t.Start();
                    return;
                }

                // Cache the category.
                _category = category;

                // Assign the session after _destroy is checked.
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
                _callback.Connected(this);
            }
            catch (SessionNotExistException)
            {
                Destroy();
            }
        }

        private void DestroyCommunicator()
        {
            Communicator? communicator;
            lock (_mutex)
            {
                communicator = _communicator;
            }
            Debug.Assert(communicator != null);
            communicator.Destroy();
        }

        private void DestroyInternal()
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
                // Expected if another thread invoked on an object from the session concurrently.
            }
            catch (SessionNotExistException)
            {
                // This can also occur.
            }
            catch (Exception e)
            {
                communicator.Logger.Warning("SessionHelper: unexpected exception when destroying the session:\n" + e);
            }

            communicator.Destroy();

            // Notify the callback that the session is gone.
            _callback.Disconnected(this);
        }

        private void ConnectImpl(Func<IRouterPrx, ISessionPrx> factory)
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
                            certificates: _certificates,
                            caCertificates: _caCertificates,
                            certificateValidationCallback: _certificateValidationCallback,
                            passwordCallback: _passwordCallback);
                    }
                }
                catch (Exception ex)
                {
                    lock (_mutex)
                    {
                        _destroy = true;
                    }
                    _callback.ConnectFailed(this, ex);
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
                        _callback.ConnectFailed(this, ex);
                        return;
                    }
                    catch (Exception ex)
                    {
                        if (finder == null)
                        {
                            _callback.ConnectFailed(this, ex);
                            return;
                        }
                        else
                        {
                            // In case of error getting router identity from RouterFinder use default identity.
                            _communicator.DefaultRouter =
                                finder.Clone(new Identity("router", "Glacier2"), Ice.IRouterPrx.Factory);
                        }
                    }
                }

                try
                {
                    _callback.CreatedCommunicator(this);
                    Ice.IRouterPrx? defaultRouter = _communicator.DefaultRouter;
                    Debug.Assert(defaultRouter != null);
                    var routerPrx = IRouterPrx.UncheckedCast(defaultRouter);
                    ISessionPrx session = factory(routerPrx);
                    Connected(routerPrx, session);
                }
                catch (Exception ex)
                {
                    _communicator.Destroy();
                    _callback.ConnectFailed(this, ex);
                }
            })).Start();
        }
    }
}
