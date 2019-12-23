//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Text;

namespace Glacier2
{

    /// <summary>
    /// A helper class for using Glacier2 with GUI applications.
    ///
    /// Applications should create a session factory for each Glacier2 router to which the application will
    /// connect. To connect with the Glacier2 router, call SessionFactory.connect. The callback object is
    /// notified of the various life cycle events. Once the session is torn down for whatever reason, the
    /// application can use the session factory to create another connection.
    /// </summary>
    public class SessionFactoryHelper
    {
        /// <summary>
        /// Creates a SessionFactory object.
        /// </summary>
        /// <param name="callback">The callback for notifications about session establishment.</param>
        /// <param name="properties">Optional properties used for communicator initialization.</param>
        /// <param name="compactIdResolver">Optional compact type ID resolver delegate used for communicator initialization.</param>
        /// <param name="dispatcher">Optional dispatcher delegate used for communicator initialization.</param>
        /// <param name="logger">Optional logger used for communicator initialization.</param>
        /// <param name="observer">Optional communicator observer used for communicator initialization.</param>
        /// <param name="threadStart">Optional thread start delegate used for communicator initialization.</param>
        /// <param name="threadStop">Optional thread stop delegate used for communicator initialization.</param>
        /// <param name="typeIdNamespaces">Optional list of TypeId namespaces used for communicator initialization.
        /// The default is Ice.TypeId.</param>
        public
        SessionFactoryHelper(SessionCallback callback,
                             Dictionary<string, string> properties,
                             Func<int, string>? compactIdResolver = null,
                             Action<Action, Ice.Connection?>? dispatcher = null,
                             Ice.Logger? logger = null,
                             Ice.Instrumentation.CommunicatorObserver? observer = null,
                             Action? threadStart = null,
                             Action? threadStop = null,
                             string[]? typeIdNamespaces = null)
        {
            _callback = callback;
            _properties = properties;
            _compactIdResolver = compactIdResolver;
            _dispatcher = dispatcher;
            _logger = logger;
            _observer = observer;
            _threadStart = threadStart;
            _threadStop = threadStop;
            _typeIdNamespaces = typeIdNamespaces;

            setDefaultProperties();
        }

        /// <summary>
        /// Set the router object identity.
        /// </summary>
        /// <param name="identity">The Glacier2 router's identity.</param>
        public void
        setRouterIdentity(Ice.Identity identity)
        {
            lock (this)
            {
                _identity = identity;
            }
        }

        /// <summary>
        /// Returns the object identity of the Glacier2 router.
        /// </summary>
        /// <returns> The Glacier2 router's identity.</returns>
        public Ice.Identity?
        getRouterIdentity()
        {
            lock (this)
            {
                return _identity;
            }
        }

        /// <summary>
        /// Sets the host on which the Glacier2 router runs.
        /// </summary>
        /// <param name="hostname">The host name (or IP address) of the router host.</param>
        public void
        setRouterHost(string hostname)
        {
            lock (this)
            {
                _routerHost = hostname;
            }
        }

        /// <summary>
        /// Returns the host on which the Glacier2 router runs.
        /// </summary>
        /// <returns>The Glacier2 router host.</returns>
        public string
        getRouterHost()
        {
            lock (this)
            {
                return _routerHost;
            }
        }

        /// <summary>
        /// Sets the protocol that will be used by the session factory to establish the connection..
        /// </summary>
        /// <param name="protocol">The protocol.</param>
        public void
        setProtocol(string protocol)
        {
            lock (this)
            {
                if (protocol == null)
                {
                    throw new ArgumentException("You must use a valid protocol");
                }

                if (!protocol.Equals("tcp") &&
                    !protocol.Equals("ssl") &&
                    !protocol.Equals("wss") &&
                    !protocol.Equals("ws"))
                {
                    throw new ArgumentException("Unknown protocol `" + protocol + "'");
                }
                _protocol = protocol;
            }
        }

        /// <summary>
        /// Returns the protocol that will be used by the session factory to establish the connection.
        /// </summary>
        /// <returns>The protocol.</returns>
        public string
        getProtocol()
        {
            lock (this)
            {
                return _protocol;
            }
        }

        /// <summary>
        /// Sets the connect and connection timeout for the Glacier2 router.
        /// </summary>
        /// <param name="timeoutMillisecs">The timeout in milliseconds. A zero
        /// or negative timeout value indicates that the router proxy has no
        /// associated timeout.</param>
        public void
        setTimeout(int timeoutMillisecs)
        {
            lock (this)
            {
                _timeout = timeoutMillisecs;
            }
        }

        /// <summary>
        /// Returns the connect and connection timeout associated with the Glacier2 router.
        /// </summary>
        /// <returns>The timeout.</returns>
        public int
        getTimeout()
        {
            lock (this)
            {
                return _timeout;
            }
        }

        /// <summary>
        /// Sets the Glacier2 router port to connect to.
        /// </summary>
        /// <param name="port">The port. If 0, then the default port (4063 for TCP or
        /// 4064 for SSL) is used.</param>
        public void
        setPort(int port)
        {
            lock (this)
            {
                _port = port;
            }
        }

        /// <summary>
        /// Returns the Glacier2 router port to connect to.
        /// </summary>
        /// <returns>The port.</returns>
        public int
        getPort()
        {
            lock (this)
            {
                return getPortInternal();
            }
        }

        private int
        getPortInternal()
        {
            return _port == 0 ? ((_protocol.Equals("ssl") ||
                                  _protocol.Equals("wss")) ? GLACIER2_SSL_PORT : GLACIER2_TCP_PORT) : _port;
        }

        /// <summary>
        /// Sets the request context to use while establishing a connection to the Glacier2 router.
        /// </summary>
        /// <param name="context">The request context.</param>
        public void
        setConnectContext(Dictionary<string, string> context)
        {
            lock (this)
            {
                _context = context;
            }
        }

        /// <summary>
        /// Determines whether the session should create an object adapter that the client
        /// can use for receiving callbacks.
        /// </summary>
        /// <param name="useCallbacks">True if the session should create an object adapter.</param>
        public void
        setUseCallbacks(bool useCallbacks)
        {
            lock (this)
            {
                _useCallbacks = useCallbacks;
            }
        }

        /// <summary>
        /// Indicates whether a newly-created session will also create an object adapter that
        /// the client can use for receiving callbaks.
        /// </summary>
        /// <returns>True if the session will create an object adapter.</returns>
        public bool
        getUseCallbacks()
        {
            lock (this)
            {
                return _useCallbacks;
            }
        }

        /// <summary>
        /// Connects to the Glacier2 router using the associated SSL credentials.
        ///
        /// Once the connection is established, SesssionCallback.connected is called on
        /// the callback object; upon failure, SessionCallback.connectFailed is called
        /// with the exception.
        /// </summary>
        /// <returns>The connected session.</returns>
        public SessionHelper
        connect()
        {
            lock (this)
            {
                SessionHelper session = new SessionHelper(_callback,
                    getRouterFinderStr(),
                    _useCallbacks,
                    CreateProperties(),
                    _compactIdResolver,
                    _dispatcher,
                    _logger,
                    _observer,
                    _threadStart,
                    _threadStop,
                    _typeIdNamespaces);
                session.connect(_context);
                return session;
            }
        }

        /// <summary>
        /// Connect the Glacier2 session using user name and password credentials.
        ///
        /// Once the connection is established, SessionCallback.connected is called on
        /// the callback object; upon failure, SessionCallback.connectFailed is called
        /// with the exception.
        /// </summary>
        /// <param name="username">The user name.</param>
        /// <param name="password">The password.</param>
        /// <returns>The connected session.</returns>
        public SessionHelper
        connect(string username, string password)
        {
            lock (this)
            {
                SessionHelper session = new SessionHelper(_callback,
                    getRouterFinderStr(),
                    _useCallbacks,
                    CreateProperties(),
                    _compactIdResolver,
                    _dispatcher,
                    _logger,
                    _observer,
                    _threadStart,
                    _threadStop,
                    _typeIdNamespaces);
                session.connect(username, password, _context);
                return session;
            }
        }

        private Dictionary<string, string>
        CreateProperties()
        {
            //
            // Clone the initialization data and properties.
            //
            Debug.Assert(_properties != null);
            var properties = new Dictionary<string, string>(_properties);

            if (!properties.ContainsKey("Ice.Default.Router") && _identity != null)
            {
                properties["Ice.Default.Router"] = createProxyStr(_identity.Value);
            }

            //
            // If using a secure connection setup the IceSSL plug-in, if IceSSL
            // plug-in has already been setup we don't want to override the
            // configuration so it can be loaded from a custom location.
            //
            if ((_protocol.Equals("ssl") || _protocol.Equals("wss")) && !properties.ContainsKey("Ice.Plugin.IceSSL"))
            {
                properties["Ice.Plugin.IceSSL"] = "IceSSL:IceSSL.PluginFactory";
            }

            return properties;
        }

        private string
        getRouterFinderStr()
        {
            return createProxyStr(new Ice.Identity("RouterFinder", "Ice"));
        }

        private string
        createProxyStr(Ice.Identity ident)
        {
            if (_timeout > 0)
            {
                return $"\"{ident}\":{_protocol} -p {getPortInternal()} -h \"{_routerHost}\" -t {_timeout}";
            }
            else
            {
                return $"\"{ident}\":{_protocol} -p {getPortInternal()} -h \"{_routerHost}\"";
            }
        }

        private void
        setDefaultProperties()
        {
            Debug.Assert(_properties != null);
            _properties["Ice.RetryIntervals"] = "-1";
        }

        private SessionCallback _callback;
        private Dictionary<string, string> _properties;
        private Func<int, string>? _compactIdResolver;
        private Action<Action, Ice.Connection?>? _dispatcher;
        private Ice.Logger? _logger;
        private Ice.Instrumentation.CommunicatorObserver? _observer;
        private Action? _threadStart;
        private Action? _threadStop;
        private string[]? _typeIdNamespaces;

        private string _routerHost = "localhost";
        private Ice.Identity? _identity = null;
        private string _protocol = "ssl";
        private int _port = 0;
        private int _timeout = 10000;
        private Dictionary<string, string>? _context;
        private bool _useCallbacks = true;
        private static int GLACIER2_SSL_PORT = 4064;
        private static int GLACIER2_TCP_PORT = 4063;
    }

}
