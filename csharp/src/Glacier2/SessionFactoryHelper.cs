//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace Glacier2
{
    /// <summary>A helper class for using Glacier2 with GUI applications.
    ///
    /// Applications should create a session factory for each Glacier2 router to which the application will connect.
    /// To connect with the Glacier2 router, call SessionFactory.connect. The callback object is notified of the
    /// various life cycle events. Once the session is torn down for whatever reason, the application can use the
    /// session factory to create another connection.</summary>
    public class SessionFactoryHelper
    {
        /// <summary>Creates a SessionFactory object.</summary>
        /// <param name="callback">The callback for notifications about session establishment.</param>
        /// <param name="properties">Optional properties used for communicator initialization.</param>
        /// <param name="logger">Optional logger used for communicator initialization.</param>
        /// <param name="observer">Optional communicator observer used for communicator initialization.</param>
        /// <param name="typeIdNamespaces">Optional list of TypeId namespaces used for communicator initialization. The
        /// default is Ice.TypeId.</param>
        public SessionFactoryHelper(ISessionCallback callback,
                             Dictionary<string, string> properties,
                             Ice.ILogger? logger = null,
                             Ice.Instrumentation.ICommunicatorObserver? observer = null,
                             string[]? typeIdNamespaces = null)
        {
            _callback = callback;
            _properties = properties;
            _logger = logger;
            _observer = observer;
            _typeIdNamespaces = typeIdNamespaces;

            SetDefaultProperties();
        }

        /// <summary>Set the router object identity.</summary>
        /// <param name="identity">The Glacier2 router's identity.</param>
        public void SetRouterIdentity(Ice.Identity identity)
        {
            lock (this)
            {
                _identity = identity;
            }
        }

        /// <summary>Returns the object identity of the Glacier2 router.</summary>
        /// <returns>The Glacier2 router's identity.</returns>
        public Ice.Identity? GetRouterIdentity()
        {
            lock (this)
            {
                return _identity;
            }
        }

        /// <summary>Sets the host on which the Glacier2 router runs.</summary>
        /// <param name="hostname">The host name (or IP address) of the router host.</param>
        public void SetRouterHost(string hostname)
        {
            lock (this)
            {
                _routerHost = hostname;
            }
        }

        /// <summary>Returns the host on which the Glacier2 router runs.</summary>
        /// <returns>The Glacier2 router host.</returns>
        public string GetRouterHost()
        {
            lock (this)
            {
                return _routerHost;
            }
        }

        /// <summary>Sets the transport that will be used by the session factory to establish the connection.</summary>
        /// <param name="transport">The transport.</param>
        public void SetTransport(string transport)
        {
            lock (this)
            {
                if (!transport.Equals("tcp") &&
                    !transport.Equals("ssl") &&
                    !transport.Equals("wss") &&
                    !transport.Equals("ws"))
                {
                    throw new ArgumentException($"unknown transport `{transport}'", nameof(transport));
                }
                _transport = transport;
            }
        }

        /// <summary>Returns the transport that will be used by the session factory to establish the connection.
        /// </summary>
        /// <returns>The transport.</returns>
        public string GetTransport()
        {
            lock (this)
            {
                return _transport;
            }
        }

        /// <summary>Sets the connect and connection timeout for the Glacier2 router.</summary>
        /// <param name="timeoutMillisecs">The timeout in milliseconds. A zero or negative timeout value indicates that
        /// the router proxy has no associated timeout.</param>
        public void SetTimeout(int timeoutMillisecs)
        {
            lock (this)
            {
                _timeout = timeoutMillisecs;
            }
        }

        /// <summary>Returns the connect and connection timeout associated with the Glacier2 router.</summary>
        /// <returns>The timeout.</returns>
        public int GetTimeout()
        {
            lock (this)
            {
                return _timeout;
            }
        }

        /// <summary>Sets the Glacier2 router port to connect to.</summary>
        /// <param name="port">The port. If 0, then the default port (4063 for TCP or 4064 for SSL) is used.</param>
        public void SetPort(int port)
        {
            lock (this)
            {
                _port = port;
            }
        }

        /// <summary>Returns the Glacier2 router port to connect to.</summary>
        /// <returns>The port.</returns>
        public int GetPort()
        {
            lock (this)
            {
                return GetPortInternal();
            }
        }

        /// <summary>Sets the request context to use while establishing a connection to the Glacier2 router.</summary>
        /// <param name="context">The request context.</param>
        public void SetConnectContext(IReadOnlyDictionary<string, string> context)
        {
            lock (this)
            {
                _context = context;
            }
        }

        /// <summary>Determines whether the session should create an object adapter that the client can use for
        /// receiving callbacks.</summary>
        /// <param name="useCallbacks">True if the session should create an object adapter.</param>
        public void SetUseCallbacks(bool useCallbacks)
        {
            lock (this)
            {
                _useCallbacks = useCallbacks;
            }
        }

        /// <summary>Indicates whether a newly-created session will also create an object adapter that the client can
        /// use for receiving callbacks.</summary>
        /// <returns>True if the session will create an object adapter.</returns>
        public bool GetUseCallbacks()
        {
            lock (this)
            {
                return _useCallbacks;
            }
        }

        /// <summary>Connects to the Glacier2 router using the associated SSL credentials.
        ///
        /// Once the connection is established, SesssionCallback.connected is called on the callback object; upon
        /// failure, SessionCallback.connectFailed is called with the exception.</summary>
        /// <returns>The connected session.</returns>
        public SessionHelper Connect()
        {
            lock (this)
            {
                var session = new SessionHelper(_callback,
                    GetRouterFinderStr(),
                    _useCallbacks,
                    CreateProperties(),
                    _logger,
                    _observer,
                    _typeIdNamespaces);
                session.Connect(_context);
                return session;
            }
        }

        /// <summary>Connect the Glacier2 session using user name and password credentials.
        ///
        /// Once the connection is established, SessionCallback.connected is called on the callback object; upon
        /// failure, SessionCallback.connectFailed is called with the exception.</summary>
        /// <param name="username">The user name.</param>
        /// <param name="password">The password.</param>
        /// <returns>The connected session.</returns>
        public SessionHelper Connect(string username, string password)
        {
            lock (this)
            {
                var session = new SessionHelper(_callback,
                    GetRouterFinderStr(),
                    _useCallbacks,
                    CreateProperties(),
                    _logger,
                    _observer,
                    _typeIdNamespaces);
                session.Connect(username, password, _context);
                return session;
            }
        }

        private int GetPortInternal()
        {
            return _port == 0 ? ((_transport.Equals("ssl") ||
                                  _transport.Equals("wss")) ? Glacier2SslPort : Glacier2TcpPort) : _port;
        }

        private Dictionary<string, string> CreateProperties()
        {
            // Clone the initialization data and properties.
            Debug.Assert(_properties != null);
            var properties = new Dictionary<string, string>(_properties);

            if (!properties.ContainsKey("Ice.Default.Router") && _identity != null)
            {
                properties["Ice.Default.Router"] = CreateProxyStr(_identity.Value);
            }

            // If using a secure connection setup the IceSSL plug-in, if IceSSL
            // plug-in has already been setup we don't want to override the
            // configuration so it can be loaded from a custom location.
            if ((_transport.Equals("ssl") || _transport.Equals("wss")) && !properties.ContainsKey("Ice.Plugin.IceSSL"))
            {
                properties["Ice.Plugin.IceSSL"] = "IceSSL:IceSSL.PluginFactory";
            }

            return properties;
        }

        private string GetRouterFinderStr() =>
            CreateProxyStr(new Ice.Identity("RouterFinder", "Ice"));

        private string CreateProxyStr(Ice.Identity ident)
        {
            if (_timeout > 0)
            {
                return $"\"{ident}\":{_transport} -p {GetPortInternal()} -h \"{_routerHost}\" -t {_timeout}";
            }
            else
            {
                return $"\"{ident}\":{_transport} -p {GetPortInternal()} -h \"{_routerHost}\"";
            }
        }

        private void SetDefaultProperties()
        {
            Debug.Assert(_properties != null);
            _properties["Ice.RetryIntervals"] = "-1";
        }

        private readonly ISessionCallback _callback;
        private readonly Dictionary<string, string> _properties;
        private readonly Ice.ILogger? _logger;
        private readonly Ice.Instrumentation.ICommunicatorObserver? _observer;
        private readonly string[]? _typeIdNamespaces;

        private string _routerHost = "localhost";
        private Ice.Identity? _identity = null;
        private string _transport = "ssl";
        private int _port = 0;
        private int _timeout = 10000;
        private IReadOnlyDictionary<string, string>? _context;
        private bool _useCallbacks = true;
        private const int Glacier2SslPort = 4064;
        private const int Glacier2TcpPort = 4063;
    }
}
