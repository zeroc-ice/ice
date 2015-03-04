// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Text;
using System.Collections.Generic;

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
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(SessionCallback callback)
    {
        _callback = callback;
        _initData = new Ice.InitializationData();
        _initData.properties = Ice.Util.createProperties();
        setDefaultProperties();
    }

    /// <summary>
    /// Creates a SessionFactory object.
    /// </summary>
    /// <param name="initData">The initialization data to use when creating the communicator.</param>
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(Ice.InitializationData initData, SessionCallback callback)
    {
        _callback = callback;
        _initData = initData;
        if(_initData.properties == null)
        {
            _initData.properties = Ice.Util.createProperties();
        }
        setDefaultProperties();
    }

    /// <summary>
    /// Creates a SessionFactory object.
    /// </summary>
    /// <param name="properties">The properties to use when creating the communicator.</param>
    /// <param name="callback">The callback object for notifications.</param>
    public
    SessionFactoryHelper(Ice.Properties properties, SessionCallback callback)
    {
        if(properties == null)
        {
            throw new Ice.InitializationException(
                                        "Attempt to create a SessionFactoryHelper with a null Properties argument");
        }
        _callback = callback;
        _initData = new Ice.InitializationData();
        _initData.properties = properties;
        setDefaultProperties();
    }

    /// <summary>
    /// Set the router object identity.
    /// </summary>
    /// <param name="identity">The Glacier2 router's identity.</param>
    public void
    setRouterIdentity(Ice.Identity identity)
    {
        lock(this)
        {
            _identity = identity;
        }
    }

    /// <summary>
    /// Returns the object identity of the Glacier2 router.
    /// </summary>
    /// <returns> The Glacier2 router's identity.</returns>
    public Ice.Identity
    getRouterIdentity()
    {
        lock(this)
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
        lock(this)
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
        lock(this)
        {
            return _routerHost;
        }
    }

    /// <summary>
    /// Sets whether to connect with the Glacier2 router securely.
    /// </summary>
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

    /// <summary>
    /// Returns whether the session factory will establish a secure connection to the Glacier2 router.
    /// </summary>
    /// <returns>The secure flag.</returns>
    public bool
    getSecure()
    {
        lock(this)
        {
            return _secure;
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
        lock(this)
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
        lock(this)
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
        lock(this)
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
        lock(this)
        {
            return _port == 0 ? (_secure ? GLACIER2_SSL_PORT : GLACIER2_TCP_PORT) : _port;
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

    /// <summary>
    /// Sets the request context to use while establishing a connection to the Glacier2 router.
    /// </summary>
    /// <param name="context">The request context.</param>
    public void
    setConnectContext(Dictionary<string, string> context)
    {
        lock(this)
        {
            _context = context;
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
        lock(this)
        {
            SessionHelper session = new SessionHelper(_callback, createInitData());
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
    connect( string username,  string password)
    {
        lock(this)
        {
            SessionHelper session = new SessionHelper(_callback, createInitData());
            session.connect(username, password, _context);
            return session;
        }
    }

    private Ice.InitializationData
    createInitData()
    {
        //
        // Clone the initialization data and properties.
        //
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
            //
            // If using a secure connection setup the IceSSL plug-in, if IceSSL
            // plug-in has already been setup we don't want to override the
            // configuration so it can be loaded from a custom location.
            //
            if(_secure && initData.properties.getProperty("Ice.Plugin.IceSSL").Length == 0)
            {
                initData.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL:IceSSL.PluginFactory");
            }
        }
        return initData;
    }

    private void
    setDefaultProperties()
    {
        _initData.properties.setProperty("Ice.ACM.Client", "0");
        _initData.properties.setProperty("Ice.RetryIntervals", "-1");
    }

    private SessionCallback _callback;
    private string _routerHost = "localhost";
    private Ice.InitializationData _initData;
    private Ice.Identity _identity = new Ice.Identity("router", "Glacier2");
    private bool _secure = true;
    private int _port = 0;
    private int _timeout = 10000;
    private Dictionary<string, string> _context;
    private static int GLACIER2_SSL_PORT = 4064;
    private static int GLACIER2_TCP_PORT = 4063;
}

}
