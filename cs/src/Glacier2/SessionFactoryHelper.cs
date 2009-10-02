// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;

namespace Glacier2
{

/// <sumary>
/// A helper class for using Glacier2 with GUI applications.
///
/// Applications should create a session factory for each Glacier2 router to which the application will
/// connect. To connect with the Glacier2 router, call SessionFactory.connect. The callback object is
/// notified of the various life cycle events. Once the session is torn down for whatever reason, the application
/// can use the session factory to create another connection.
/// </sumary>
public class SessionFactoryHelper
{
    /// <sumary>
    /// A callback class to get notifications of status changes in the Glacier2 session.
    /// All callbacks on the <code>Callback</code> interface occur in the main swing thread.
    /// </sumary>
    public interface Callback
    {
        /// <sumary>
        /// Notifies the application that the communicator was created.
        /// </sumary>
        /// <param name="session">The Glacier2 session.</param>
        void
        createdCommunicator(SessionHelper session);

        /// <sumary>
        /// Notifies the application that the Glacier2 session has been established.
        /// </sumary>
        /// <param name="session">The established session.</param>
        void
        connected(SessionHelper session);
        
        /// <sumary>
        /// Notifies the application that the Glacier2 session has been disconnected.
        /// </sumary>
        /// <param name="session">The disconnected session.</param>
        void
        disconnected(SessionHelper session);

        /// <sumary>
        /// Notifies the application that the Glacier2 session establishment failed. 
        /// </sumary>
        /// <param name="session">The session reporting the connection failure.</param>
        /// <param name="ex">The exception.</param>
        void
        connectFailed(SessionHelper session, Exception ex);

        System.Windows.Threading.Dispatcher
        getDispatcher();
    }

    /// <sumary>
    /// Creates a SessionFactory object.
    /// </sumary>
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(Callback callback)
    {
        _callback = callback;
        _initData = new Ice.InitializationData();
        _initData.properties = Ice.Util.createProperties();
        setDefaultProperties();
    }

    /// <sumary>
    /// Creates a SessionFactory object.
    /// </sumary>
    /// <param name="initData">The initialization data to use when creating the communicator.</param>
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(Ice.InitializationData initData, Callback callback)
    {
        _callback = callback;
        _initData = initData;
        setDefaultProperties();
    }

    /// <sumary>
    /// Creates a SessionFactory object.
    /// </sumary>
    /// <param name="properties">The properties to use when creating the communicator.</param>
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(Ice.Properties properties, Callback callback)
    {
        _callback = callback;
        _initData = new Ice.InitializationData();
        _initData.properties = properties;
        setDefaultProperties();
    }

    /// <sumary>
    /// Set the router object identity.
    /// </sumary>
    public void
    setRouterIdentity(Ice.Identity identity)
    {
        lock(this)
        {
            _identity = identity;
        }
    }

    /// <sumary>
    /// Returns the object identity of the Glacier2 router.
    /// </sumary>
    /// <returns> The Glacier2 router's identity.</returns>
    public Ice.Identity
    getRouterIdentity()
    {
        lock(this)
        {
            return _identity;
        }
    }

    /// <sumary>
    /// Sets the host on which the Glacier2 router runs.
    /// </sumary>
    /// <param name="hostname">The host name (or IP address) of the router host.</param>
    public void
    setRouterHost(string hostname)
    {
        lock(this)
        {
            _routerHost = hostname;
        }
    }

    /// <sumary>
    /// Returns the host on which the Glacier2 router runs.
    /// </sumary>
    /// <returns>The Glacier2 router host.</returns>
    public string
    getRouterHost()
    {
        lock(this)
        {
            return _routerHost;
        }
    }

    /// <sumary>
    /// Sets whether to connect with the Glacier2 router securely.
    /// </sumary>
    /// <param name="secure">If true, the client connects to the router
    /// via SSL; otherwise, the client connects via TCP.</param>
    public void
    setSecure(bool secure)
    {
        lock(this)
        {
            _secure = secure;
        }
    }

    /// <sumary>
    /// Returns whether the session factory will establish a secure connection to the Glacier2 router.
    /// </sumary>
    /// <returns>The secure flag.</returns>
    public bool
    getSecure()
    {
        lock(this)
        {
            return _secure;
        }
    }

    /// <sumary>
    /// Sets the connect and connection timeout for the Glacier2 router.
    /// </sumary>
    /// <param name="timeoutMillisecs">The timeout in milliseconds. A zero
    /// or negative timeout value indicates that the router proxy has no
    /// associated timeout.</param>
    public void
    setTimeout(int timeoutMillisecs)
    {
        lock(this)
        {
            _timeout = timeoutMillisecs;
        }
    }

    /// <sumary>
    /// Returns the connect and connection timeout associated with the Glacier2 router.
    /// </sumary>
    /// <returns>The timeout.</returns>
    public int
    getTimeout()
    {
        lock(this)
        {
            return _timeout;
        }
    }

    /// <sumary>
    /// Sets the Glacier2 router port to connect to.
    /// </sumary>
    /// <param name="port">The port. If 0, then the default port (4063 for TCP or
    /// 4064 for SSL) is used.</param>
    public void
    setPort(int port)
    {
        lock(this)
        {
            _port = port;
        }
    }

    /// <sumary>
    /// Returns the Glacier2 router port to connect to.
    /// </sumary>
    /// <returns>The port.</returns>
    public int
    getPort()
    {
        lock(this)
        {
            return _port == 0 ? (_secure ? GLACIER2_TCP_PORT : GLACIER2_SSL_PORT) : _port;
        }
    }

    /**
     * Returns the initialization data used to initialize the communicator.
     *
     * @return The initialization data.
     */
    public Ice.InitializationData
    getInitializationData()
    {
        lock(this)
        {
            return _initData;
        }
    }

    /// <sumary>
    /// Connects to the Glacier2 router using the associated SSL credentials.
    /// 
    /// Once the connection is established, Callback.connected is called on
    /// the callback object; upon failure, Callback.connectFailed is called 
    /// with the exception.
    /// </sumary>
    /// <returns>The connected session.</returns>
    public SessionHelper
    connect()
    {
        lock(this)
        {
            SessionHelper session = new SessionHelper(_callback, createInitData());
            session.connect();
            return session;
        }
    }

    /// <sumary>
    /// Connect the Glacier2 session using user name and password credentials.
    ///
    /// Once the connection is established, Callback.connected is called on
    /// the callback object; upon failure, Callback.connectFailed is called
    /// with the exception. 
    /// </sumary>
    /// <param name="username">The user name.</param>
    /// <param name="password">The password.</param>
    /// <returns>The connected session.</returns>
    public SessionHelper
    connect( string username,  string password)
    {
        lock(this)
        {
            SessionHelper session = new SessionHelper(_callback, createInitData());
            session.connect(username, password);
            return session;
        }
    }

    private Ice.InitializationData
    createInitData()
    {
        // Clone the initialization data and properties.
        Ice.InitializationData initData = (Ice.InitializationData)_initData.Clone();
        initData.properties = initData.properties.ice_clone_();

        if(initData.properties.getProperty("Ice.Default.Router").Length == 0)
        {
            StringBuilder sb = new StringBuilder();
            sb.Append("\"");
            sb.Append(Ice.Util.identityToString(_identity));
            sb.Append("\"");
            sb.Append(":");
            if(_secure)
            {
                sb.Append("ssl -p ");
            }
            else
            {
                sb.Append("tcp -p ");
            }
            if(_port != 0)
            {
                sb.Append(_port);
            }
            else
            {
                if(_secure)
                {
                    sb.Append(GLACIER2_SSL_PORT);
                }
                else
                {
                    sb.Append(GLACIER2_TCP_PORT);
                }
            }

            sb.Append(" -h ");
            sb.Append(_routerHost);
            if(_timeout > 0)
            {
                sb.Append(" -t ");
                sb.Append(_timeout);
            }
            initData.properties.setProperty("Ice.Default.Router", sb.ToString());
        }
        return initData;
    }

    private void
    setDefaultProperties()
    {
        _initData.properties.setProperty("Ice.ACM.Client", "0");
        _initData.properties.setProperty("Ice.RetryIntervals", "-1");
    }

    private Callback _callback;
    private string _routerHost = "127.0.0.1";
    private Ice.InitializationData _initData;
    private Ice.Identity _identity = new Ice.Identity("router", "Glacier2");
    private bool _secure = true;
    private int _port = 0;
    private int _timeout = 10000;
    private static int GLACIER2_SSL_PORT = 4064;
    private static int GLACIER2_TCP_PORT = 4063;
}

}
