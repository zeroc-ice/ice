//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;
using ZeroC.Ice;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Glacier2
{
    /// <summary>A helper class for using Glacier2 with GUI applications.
    ///
    /// Applications should create a session factory for each Glacier2 router to which the application will connect.
    /// To connect with the Glacier2 router, call SessionFactory.connect. The callback object is notified of the
    /// various life cycle events. Once the session is torn down for whatever reason, the application can use the
    /// session factory to create another connection.</summary>
    public class SessionFactoryHelper
    {
        /// <summary>Gets or sets the request context to use while establishing a connection to the Glacier2 router.
        /// </summary>
        public IReadOnlyDictionary<string, string>? ConnectContext
        {
            get
            {
                lock (_mutex)
                {
                    return _context;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _context = value;
                }
            }
        }

        /// <summary>Gets or sets the Glacier2 router port to connect to. If 0, then the default port (4063 for TCP or
        /// 4064 for SSL) is used</summary>
        public int Port
        {
            get
            {
                lock (_mutex)
                {
                    return _port;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _port = value;
                }
            }
        }

        /// <summary>Gets or sets the host on which the Glacier2 router runs.</summary>
        public string RouterHost
        {
            get
            {
                lock (_mutex)
                {
                    return _routerHost;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _routerHost = value;
                }
            }
        }

        /// <summary>Gets or sets the router object identity.</summary>
        public Identity? RouterIdentity
        {
            get
            {
                lock (_mutex)
                {
                    return _identity;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _identity = value;
                }
            }
        }

        /// <summary>Gets or sets the connect and connection timeout for the Glacier2 router, a zero or negative
        /// timeout value indicates that.</summary>
        public int Timeout
        {
            get
            {
                lock (_mutex)
                {
                    return _timeout;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _timeout = value;
                }
            }
        }

        /// <summary>Gets or sets the transport that will be used by the session factory to establish the connection.
        /// </summary>
        public string Transport
        {
            get
            {
                lock (_mutex)
                {
                    return _transport;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _transport = value;
                }
            }
        }

        /// <summary>Get or sets whether the session should create an object adapter that the client can use for
        /// receiving callbacks.</summary>
        public bool UseCallbacks
        {
            get
            {
                lock (_mutex)
                {
                    return _useCallbacks;
                }
            }
            set
            {
                lock (_mutex)
                {
                    _useCallbacks = value;
                }
            }
        }

        private int PortInternal
        {
            get
            {
                lock (_mutex)
                {
                    if (_port == 0)
                    {
                        return _transport == "ssl" || _transport == "wss" ? Glacier2SslPort : Glacier2TcpPort;
                    }
                    else
                    {
                        return _port;
                    }
                }
            }
        }

        private const int Glacier2SslPort = 4064;
        private const int Glacier2TcpPort = 4063;

        private readonly ISessionCallback _callback;
        private readonly X509Certificate2Collection? _caCertificates;
        private readonly X509Certificate2Collection? _certificates;
        private readonly RemoteCertificateValidationCallback? _certificateValidationCallback;
        private IReadOnlyDictionary<string, string>? _context;
        private Identity? _identity = null;
        private readonly ILogger? _logger;
        private readonly object _mutex = new object();
        private readonly ICommunicatorObserver? _observer;
        private readonly IPasswordCallback? _passwordCallback;
        private int _port = 0;
        private readonly Dictionary<string, string> _properties;
        private string _routerHost = "localhost";
        private int _timeout = 10000;
        private string _transport = "ssl";
        private bool _useCallbacks = true;

        /// <summary>Creates a SessionFactory object.</summary>
        /// <param name="callback">The callback for notifications about session establishment.</param>
        /// <param name="properties">Optional properties used for communicator initialization.</param>
        /// <param name="logger">Optional logger used for communicator initialization.</param>
        /// <param name="observer">Optional communicator observer used for communicator initialization.</param>
        /// <param name="certificates">Optional certificates used by secure transports.</param>
        /// <param name="caCertificates">Optional CA certificates used by secure transports.</param>
        /// <param name="certificateValidationCallback">Optional certificate validation callback used by secure
        /// transports.</param>
        /// <param name="passwordCallback">Optional password callback used by secure transports.</param>
        public SessionFactoryHelper(
            ISessionCallback callback,
            Dictionary<string, string> properties,
            ILogger? logger = null,
            ICommunicatorObserver? observer = null,
            X509Certificate2Collection? certificates = null,
            X509Certificate2Collection? caCertificates = null,
            RemoteCertificateValidationCallback? certificateValidationCallback = null,
            IPasswordCallback? passwordCallback = null)
        {
            _callback = callback;
            _properties = properties;
            _logger = logger;
            _observer = observer;
            _certificates = certificates;
            _caCertificates = caCertificates;
            _certificateValidationCallback = certificateValidationCallback;
            _passwordCallback = passwordCallback;

            _properties["Ice.RetryIntervals"] = "-1";
        }

        /// <summary>Connects to the Glacier2 router using the associated SSL credentials.
        ///
        /// Once the connection is established, SesssionCallback.connected is called on the callback object; upon
        /// failure, SessionCallback.connectFailed is called with the exception.</summary>
        /// <returns>The connected session.</returns>
        public SessionHelper Connect()
        {
            lock (_mutex)
            {
                var session = new SessionHelper(
                    _callback,
                    CreateProxyStr(new Identity("RouterFinder", "Ice")),
                    _useCallbacks,
                    CreateProperties(),
                    _logger,
                    _observer,
                    _certificates,
                    _caCertificates,
                    _certificateValidationCallback,
                    _passwordCallback);
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
            lock (_mutex)
            {
                var session = new SessionHelper(_callback,
                    CreateProxyStr(new Identity("RouterFinder", "Ice")),
                    _useCallbacks,
                    CreateProperties(),
                    _logger,
                    _observer,
                    _certificates,
                    _caCertificates,
                    _certificateValidationCallback,
                    _passwordCallback);
                session.Connect(username, password, _context);
                return session;
            }
        }

        private Dictionary<string, string> CreateProperties()
        {
            // Clone the initialization properties.
            Debug.Assert(_properties != null);
            var properties = new Dictionary<string, string>(_properties);

            if (!properties.ContainsKey("Ice.Default.Router") && _identity is Identity identity)
            {
                properties["Ice.Default.Router"] = CreateProxyStr(identity);
            }
            return properties;
        }

        private string CreateProxyStr(Identity identity) => _timeout > 0 ?
            $"\"{identity}\":{_transport} -p {PortInternal} -h \"{_routerHost}\" -t {_timeout}" :
            $"\"{identity}\":{_transport} -p {PortInternal} -h \"{_routerHost}\"";
    }
}
