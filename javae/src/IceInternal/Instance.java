// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class Instance
{
    public Ice.InitializationData
    initializationData()
    {
	//
	// No check for destruction. It must be possible to access the
	// initialization data after destruction.
	//
	// No mutex lock, immutable.
	//
        return _initData;
    }

    public TraceLevels
    traceLevels()
    {
	// No mutex lock, immutable.
        return _traceLevels;
    }

    public DefaultsAndOverrides
    defaultsAndOverrides()
    {
	// No mutex lock, immutable.
        return _defaultsAndOverrides;
    }

    public synchronized RouterManager
    routerManager()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _routerManager;
    }

    public synchronized LocatorManager
    locatorManager()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _locatorManager;
    }

    public synchronized ReferenceFactory
    referenceFactory()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _referenceFactory;
    }

    public synchronized ProxyFactory
    proxyFactory()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _proxyFactory;
    }

    public synchronized OutgoingConnectionFactory
    outgoingConnectionFactory()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _outgoingConnectionFactory;
    }

    public synchronized ObjectAdapterFactory
    objectAdapterFactory()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _objectAdapterFactory;
    }

    public int
    threadPerConnectionStackSize()
    {
	return _threadPerConnectionStackSize;
    }

    public synchronized EndpointFactory
    endpointFactory()
    {
	if(_state == StateDestroyed)
	{
	    throw new Ice.CommunicatorDestroyedException();
	}

        return _endpointFactory;
    }

    public int
    messageSizeMax()
    {
        // No mutex lock, immutable.
	return _messageSizeMax;
    }

    public void
    flushBatchRequests()
    {
	OutgoingConnectionFactory connectionFactory;
	ObjectAdapterFactory adapterFactory;

	synchronized(this)
	{
	    if(_state == StateDestroyed)
	    {
		throw new Ice.CommunicatorDestroyedException();
	    }
	    
	    connectionFactory = _outgoingConnectionFactory;
	    adapterFactory = _objectAdapterFactory;
	}

	connectionFactory.flushBatchRequests();
	adapterFactory.flushBatchRequests();
    }

    public Ice.Identity
    stringToIdentity(String s)
    {
        Ice.Identity ident = new Ice.Identity();

        //
        // Find unescaped separator.
        //
        int slash = -1, pos = 0;
        while((pos = s.indexOf('/', pos)) != -1)
        {
            if(pos == 0 || s.charAt(pos - 1) != '\\')
            {
                if(slash == -1)
                {
                    slash = pos;
                }
                else
                {
                    //
                    // Extra unescaped slash found.
                    //
                    Ice.IdentityParseException ex = new Ice.IdentityParseException();
                    ex.str = s;
                    throw ex;
                }
            }
            pos++;
        }

        if(slash == -1)
        {
            Ice.StringHolder token = new Ice.StringHolder();
            if(!IceUtil.StringUtil.unescapeString(s, 0, s.length(), token))
            {
                Ice.IdentityParseException ex = new Ice.IdentityParseException();
                ex.str = s;
                throw ex;
            }
            ident.category = "";
            ident.name = token.value;
        }
        else
        {
            Ice.StringHolder token = new Ice.StringHolder();
            if(!IceUtil.StringUtil.unescapeString(s, 0, slash, token))
            {
                Ice.IdentityParseException ex = new Ice.IdentityParseException();
                ex.str = s;
                throw ex;
            }
            ident.category = token.value;
            if(slash + 1 < s.length())
            {
                if(!IceUtil.StringUtil.unescapeString(s, slash + 1, s.length(), token))
                {
                    Ice.IdentityParseException ex = new Ice.IdentityParseException();
                    ex.str = s;
                    throw ex;
                }
                ident.name = token.value;
            }
            else
            {
                ident.name = "";
            }
        }

        return ident;
    }

    public String
    identityToString(Ice.Identity ident)
    {
        if(ident.category.length() == 0)
        {
            return IceUtil.StringUtil.escapeString(ident.name, "/");
        }
        else
        {
            return IceUtil.StringUtil.escapeString(ident.category, "/") + '/' +
                IceUtil.StringUtil.escapeString(ident.name, "/");
        }
    }

    //
    // Only for use by Ice.Communicator
    //
    public
    Instance(Ice.Communicator communicator, Ice.InitializationData initData)
    {
        _state = StateActive;
        _initData = initData;

        try
        {
	    if(_initData.logger == null)
	    {
	        _initData.logger = new Ice.LoggerI(_initData.properties.getProperty("Ice.ProgramName"));
	    }

            validatePackages();

            _traceLevels = new TraceLevels(_initData.properties);

            _defaultsAndOverrides = new DefaultsAndOverrides(_initData.properties);

	    {
		final int defaultMessageSizeMax = 1024;
		int num = _initData.properties.getPropertyAsIntWithDefault("Ice.MessageSizeMax", defaultMessageSizeMax);
		if(num < 1)
		{
		    _messageSizeMax = defaultMessageSizeMax * 1024; // Ignore non-sensical values.
		}
		else if(num > 0x7fffffff / 1024)
		{
		    _messageSizeMax = 0x7fffffff;
		}
		else
		{
		    _messageSizeMax = num * 1024; // Property is in kilobytes, _messageSizeMax in bytes
		}
	    }

	    {
		int stackSize = _initData.properties.getPropertyAsInt("Ice.ThreadPerConnection.StackSize");
		if(stackSize < 0)
		{
		    stackSize = 0;
		}
		_threadPerConnectionStackSize = stackSize;
	    }

            _routerManager = new RouterManager();

            _locatorManager = new LocatorManager();

            _referenceFactory = new ReferenceFactory(this, communicator);

            _proxyFactory = new ProxyFactory(this);

            _endpointFactory = new EndpointFactory(this);

            _outgoingConnectionFactory = new OutgoingConnectionFactory(this);

            _objectAdapterFactory = new ObjectAdapterFactory(this, communicator);
        }
        catch(Ice.LocalException ex)
        {
            destroy();
            throw ex;
        }
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtil.Debug.FinalizerAssert(_state == StateDestroyed);
        IceUtil.Debug.FinalizerAssert(_referenceFactory == null);
        IceUtil.Debug.FinalizerAssert(_proxyFactory == null);
        IceUtil.Debug.FinalizerAssert(_outgoingConnectionFactory == null);
        IceUtil.Debug.FinalizerAssert(_objectAdapterFactory == null);
        IceUtil.Debug.FinalizerAssert(_routerManager == null);
        IceUtil.Debug.FinalizerAssert(_locatorManager == null);
        IceUtil.Debug.FinalizerAssert(_endpointFactory == null);

	//
	// Do not call parent's finalizer, CLDC Object does not have it.
	//
    }
    
    public void
    finishSetup(Ice.StringSeqHolder args)
    {
	//
	// Get default router and locator proxies. 
	//
	if(_defaultsAndOverrides.defaultRouter.length() > 0)
	{
	    _referenceFactory.setDefaultRouter(Ice.RouterPrxHelper.uncheckedCast(
		    _proxyFactory.stringToProxy(_defaultsAndOverrides.defaultRouter)));
	}

	if(_defaultsAndOverrides.defaultLocator.length() > 0)
	{
	    _referenceFactory.setDefaultLocator(Ice.LocatorPrxHelper.uncheckedCast(
		    _proxyFactory.stringToProxy(_defaultsAndOverrides.defaultLocator)));
	}
    }

    //
    // Only for use by Ice.Communicator
    //
    public void
    destroy()
    {
	synchronized(this)
	{
	    //
	    // If the _state is not StateActive then the instance is
	    // either being destroyed, or has already been destroyed.
	    //
	    if(_state != StateActive)
	    {
		return;
	    }
	    
	    //
	    // We cannot set state to StateDestroyed otherwise instance
	    // methods called during the destroy process (such as
	    // outgoingConnectionFactory() from
	    // ObjectAdapterI::deactivate() will cause an exception.
	    //
	    _state = StateDestroyInProgress;
	}

        if(_objectAdapterFactory != null)
        {
            _objectAdapterFactory.shutdown();
        }

        if(_outgoingConnectionFactory != null)
        {
            _outgoingConnectionFactory.destroy();
        }

        if(_objectAdapterFactory != null)
        {
            _objectAdapterFactory.destroy();
        }
	
        if(_outgoingConnectionFactory != null)
        {
            _outgoingConnectionFactory.waitUntilFinished();
        }
	
	synchronized(this)
	{
	    _objectAdapterFactory = null;

	    _outgoingConnectionFactory = null;

            if(_referenceFactory != null)
            {
                _referenceFactory.destroy();
                _referenceFactory = null;
            }
	    
	    // No destroy function defined.
	    // _proxyFactory.destroy();
	    _proxyFactory = null;

            if(_routerManager != null)
            {
                _routerManager.destroy();
                _routerManager = null;
            }

            if(_locatorManager != null)
            {
                _locatorManager.destroy();
                _locatorManager = null;
            }

            if(_endpointFactory != null)
            {
                _endpointFactory.destroy();
                _endpointFactory = null;
            }
	    
	    _state = StateDestroyed;
	}
    }

    private void
    validatePackages()
    {
        final String prefix = "Ice.Package.";
        java.util.Hashtable map = _initData.properties.getPropertiesForPrefix(prefix);
        java.util.Enumeration p = map.keys();
        while(p.hasMoreElements())
        {
            String key = (String)p.nextElement();
            String pkg = (String)map.get(key);
            if(key.length() == prefix.length())
            {
                _initData.logger.warning("ignoring invalid property: " + key + "=" + pkg);
            }
            String module = key.substring(prefix.length());
            String className = pkg + "." + module + "._Marker";
            try
            {
                Class.forName(className);
            }
            catch(java.lang.Exception ex)
            {
                _initData.logger.warning("unable to validate package: " + key + "=" + pkg);
            }
        }
    }

    private static final int StateActive = 0;
    private static final int StateDestroyInProgress = 1;
    private static final int StateDestroyed = 2;
    private int _state;

    private final Ice.InitializationData _initData; // Immutable, not reset by destroy().
    private final TraceLevels _traceLevels; // Immutable, not reset by destroy().
    private final DefaultsAndOverrides _defaultsAndOverrides; // Immutable, not reset by destroy().
    private final int _messageSizeMax; // Immutable, not reset by destroy().
    private RouterManager _routerManager;
    private LocatorManager _locatorManager;
    private ReferenceFactory _referenceFactory;
    private ProxyFactory _proxyFactory;
    private OutgoingConnectionFactory _outgoingConnectionFactory;
    private ObjectAdapterFactory _objectAdapterFactory;
    private final int _threadPerConnectionStackSize;
    private EndpointFactory _endpointFactory;

    private static boolean _oneOffDone = false;
}
