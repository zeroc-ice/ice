// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Glacier2;

/**
 * A helper class for using Glacier2 with GUI applications.
 *
 * Applications should create a session factory for each Glacier2 router to which the application will
 * connect. To connect with the Glacier2 router, call {@link SessionFactoryHelper#connect}. The callback object is
 * notified of the various life cycle events. Once the session is torn down for whatever reason, the application
 * can use the session factory to create another connection.
 */
public class SessionFactoryHelper
{
    /**
     * Creates a SessionFactory object.
     *
     * @param callback The callback object for notifications.
     * @throws Ice.InitializationException If a failure occurred while initializing the communicator.
     */
    public
    SessionFactoryHelper(SessionCallback callback)
        throws Ice.InitializationException
    {
        initialize(callback, new Ice.InitializationData(), Ice.Util.createProperties());
    }

    /**
     * Creates a SessionFactory object.
     *
     * @param initData The initialization data to use when creating the communicator.
     * @param callback The callback object for notifications.
     * @throws Ice.InitializationException If a failure occurred while initializing the communicator.
     */
    public
    SessionFactoryHelper(Ice.InitializationData initData, SessionCallback callback)
        throws Ice.InitializationException
    {
        initialize(callback, initData, initData.properties);
    }

    /**
     * Creates a SessionFactory object.
     *
     * @param properties The properties to use when creating the communicator.
     * @param callback The callback object for notifications.
     * @throws Ice.InitializationException If a failure occurred while initializing the communicator.
     */
    public
    SessionFactoryHelper(Ice.Properties properties, SessionCallback callback)
        throws Ice.InitializationException
    {
        initialize(callback, new Ice.InitializationData(), properties);
    }

    private void
    initialize(SessionCallback callback, Ice.InitializationData initData, Ice.Properties properties)
        throws Ice.InitializationException
    {
        if(callback == null)
        {
            throw new Ice.InitializationException("Attempt to create a SessionFactoryHelper with a null " +
                                                  "SessionCallback argument");
        }

        if(initData == null)
        {
            throw new Ice.InitializationException("Attempt to create a SessionFactoryHelper with a null " +
                                                  "InitializationData argument");
        }

        if(properties == null)
        {
            throw new Ice.InitializationException("Attempt to create a SessionFactoryHelper with a null Properties " +
                                                  "argument");
        }

        _callback = callback;
        _initData = initData;
        _initData.properties = properties;

        //
        // Set default properties;
        //
        _initData.properties.setProperty("Ice.RetryIntervals", "-1");
    }

    /**
     * Set the router object identity.
     *
     * @param identity The Glacier2 router's identity.
     */
    synchronized public void
    setRouterIdentity(Ice.Identity identity)
    {
        _identity = identity;
    }

    /**
     * Returns the object identity of the Glacier2 router.
     *
     * @return The Glacier2 router's identity.
     */
    synchronized public Ice.Identity
    getRouterIdentity()
    {
        return _identity;
    }

    /**
     * Sets the host on which the Glacier2 router runs.
     *
     * @param hostname The host name (or IP address) of the router host.
     */
    synchronized public void
    setRouterHost(String hostname)
    {
        _routerHost = hostname;
    }

    /**
     * Returns the host on which the Glacier2 router runs.
     *
     * @return The Glacier2 router host.
     */
    synchronized public String
    getRouterHost()
    {
        return _routerHost;
    }

    /**
     * Sets whether to connect with the Glacier2 router securely.
     *
     * @param secure If <code>true</code>, the client connects to the router
     * via SSL; otherwise, the client connects via TCP.
     * @deprecated deprecated, use SessionFactoryHelper.setProtocol instead
     */
    @Deprecated
    public void
    setSecure(boolean secure)
    {
        setProtocol(secure ? "ssl" : "tcp");
    }

    /**
     * Returns whether the session factory will establish a secure connection to the Glacier2 router.
     *
     * @return The secure flag.
     * @deprecated deprecated, use SessionFactoryHelper.getProtocol instead
     */
    @Deprecated
    public boolean
    getSecure()
    {
        return getProtocol().equals("ssl");
    }

    /**
     *
     * Sets the protocol that will be used by the session factory to establish the connection.
     *
     * @param protocol The communication protocol.
     */
     synchronized public void setProtocol(String protocol)
     {
        if(protocol == null)
        {
            throw new IllegalArgumentException("You must use a valid protocol");
        }

        if(!protocol.equals("tcp") &&
           !protocol.equals("ssl") &&
           !protocol.equals("wss") &&
           !protocol.equals("ws"))
        {
            throw new IllegalArgumentException("Unknow protocol `" + protocol + "'");
        }

        _protocol = protocol;
     }

    /**
     *
     * Returns the protocol that will be used by the session factory to establish the connection.
     *
     * @return The protocol.
     */
     synchronized public String getProtocol()
     {
        return _protocol;
     }

    /**
     * Sets the connect and connection timeout for the Glacier2 router.
     *
     * @param timeoutMillisecs The timeout in milliseconds. A zero
     * or negative timeout value indicates that the router proxy has no associated timeout.
     */
    synchronized public void
    setTimeout(int timeoutMillisecs)
    {
        _timeout = timeoutMillisecs;
    }

    /**
     * Returns the connect and connection timeout associated with the Glacier2 router.
     *
     * @return The timeout.
     */
    synchronized public int
    getTimeout()
    {
        return _timeout;
    }

    /**
     * Sets the Glacier2 router port to connect to.
     *
     * @param port The port. If 0, then the default port (4063 for TCP or 4064 for SSL) is used.
     */
    synchronized public void
    setPort(int port)
    {
        _port = port;
    }

    /**
     * Returns the Glacier2 router port to connect to.
     *
     * @return The port.
     */
    synchronized public int
    getPort()
    {
        return getPortInternal();
    }

    private int getPortInternal()
    {
        return _port == 0 ? ((_protocol.equals("ssl") ||
                              _protocol.equals("wss"))? GLACIER2_SSL_PORT : GLACIER2_TCP_PORT) : _port;
    }

    /**
     * Returns the initialization data used to initialize the communicator.
     *
     * @return The initialization data.
     */
    synchronized public Ice.InitializationData
    getInitializationData()
    {
        return _initData;
    }

    /**
     * Sets the request context to use while establishing a connection to the Glacier2 router.
     *
     * @param context The request context.
     */
    synchronized public void
    setConnectContext(final java.util.Map<String, String> context)
    {
        _context = context;
    }

    /**
     * Determines whether the session should create an object adapter that the client
     * can use for receiving callbacks.
     *
     * @param useCallbacks True if the session should create an object adapter.
     */
    synchronized public void
    setUseCallbacks(boolean useCallbacks)
    {
        _useCallbacks = useCallbacks;
    }

    /**
     * Indicates whether a newly-created session will also create an object adapter that
     * the client can use for receiving callbacks.
     *
     * @return True if the session will create an object adapter.
     */
    synchronized public boolean
    getUseCallbacks()
    {
        return _useCallbacks;
    }

    /**
     * Connects to the Glacier2 router using the associated SSL credentials.
     *
     * Once the connection is established, {@link SessionCallback#connected} is called on the callback object;
     * upon failure, {@link SessionCallback#connectFailed} is called with the exception.
     *
     * @return The connected session.
     */
    synchronized public SessionHelper
    connect()
    {
        SessionHelper session = new SessionHelper(_callback, createInitData(), getRouterFinderStr(), _useCallbacks);
        session.connect(_context);
        return session;
    }

    /**
     * Connect the Glacier2 session using user name and password credentials.
     *
     * Once the connection is established, {@link SessionCallback#connected} is called on the callback object;
     * upon failure, {@link SessionCallback#connectFailed} is called with the exception.
     *
     * @param username The user name.
     * @param password The password.
     * @return The connected session.
     */
    synchronized public SessionHelper
    connect(final String username, final String password)
    {
        SessionHelper session = new SessionHelper(_callback, createInitData(), getRouterFinderStr(), _useCallbacks);
        session.connect(username, password, _context);
        return session;
    }

    private Ice.InitializationData
    createInitData()
    {
        //
        // Clone the initialization data and properties.
        //
        Ice.InitializationData initData = _initData.clone();
        initData.properties = initData.properties._clone();

        if(initData.properties.getProperty("Ice.Default.Router").length() == 0 && _identity != null)
        {
            initData.properties.setProperty("Ice.Default.Router", getProxyStr(_identity));
        }

        //
        // If using a secure connection setup the IceSSL plug-in, if IceSSL
        // plug-in has already been setup we don't want to override the
        // configuration so it can be loaded from a custom location.
        //
        if((_protocol.equals("ssl") || _protocol.equals("wss")) &&
           initData.properties.getProperty("Ice.Plugin.IceSSL").length() == 0)
        {
            initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        }

        return initData;
    }

    private String
    getRouterFinderStr()
    {
        Ice.Identity ident = new Ice.Identity("RouterFinder", "Ice");
        return getProxyStr(ident);
    }

    private String
    getProxyStr(Ice.Identity ident)
    {
        StringBuilder sb = new StringBuilder();
        sb.append("\"");
        sb.append(Ice.Util.identityToString(ident));
        sb.append("\":");
        sb.append(_protocol + " -p ");
        sb.append(getPortInternal());
        sb.append(" -h \"");
        sb.append(_routerHost);
        sb.append("\"");
        if(_timeout > 0)
        {
            sb.append(" -t ");
            sb.append(_timeout);
        }
        return sb.toString();
    }

    private SessionCallback _callback;
    private String _routerHost = "localhost";
    private Ice.InitializationData _initData;
    private Ice.Identity _identity = null;
    private String _protocol = "ssl";
    private int _port = 0;
    private int _timeout = 10000;
    private java.util.Map<String, String> _context;
    private boolean _useCallbacks = true;
    private static final int GLACIER2_SSL_PORT = 4064;
    private static final int GLACIER2_TCP_PORT = 4063;
}
