// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class Util
{
    public static Properties
    createProperties()
    {
        return new PropertiesI();
    }

    public static Properties
    createProperties(StringSeqHolder args)
    {
        return new PropertiesI(args);
    }

    public static Properties
    createProperties(String[] args)
    {
	StringSeqHolder argsH = new StringSeqHolder(args);
        return createProperties(argsH);
    }

    public static Properties
    getDefaultProperties()
    {
	synchronized(_defaultPropertiesMutex)
	{
	    if(_defaultProperties == null)
	    {
		_defaultProperties = createProperties();
	    }
	    return _defaultProperties;
	}
    }

    public static Properties
    getDefaultProperties(StringSeqHolder args)
    {
	synchronized(_defaultPropertiesMutex)
	{
	    if(_defaultProperties == null)
	    {
		_defaultProperties = createProperties(args);
	    }
	    return _defaultProperties;
	}
    }

    public static Properties
    getDefaultProperties(String[] args)
    {
	StringSeqHolder argsH = new StringSeqHolder(args);
	return getDefaultProperties(argsH);
    }

    public static Communicator
    initialize(StringSeqHolder args)
    {
    	InitializationData initData = new InitializationData();
	return initialize(args, initData);
    }

    public static Communicator
    initialize(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initialize(argsH);
    }

    public static Communicator
    initialize(StringSeqHolder args, InitializationData initData)
    {
    	if(initData.properties == null)
	{
	    initData.properties = getDefaultProperties(args);
	}
	args.value = initData.properties.parseIceCommandLineOptions(args.value);

        CommunicatorI result = new CommunicatorI(initData);
        result.finishSetup(args);
        return result;
    }

    public static Communicator
    initialize(String[] args, InitializationData initData)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initialize(argsH, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithLogger(StringSeqHolder args, Logger logger)
    {
    	InitializationData initData = new InitializationData();
	initData.logger = logger;
	return initialize(args, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithLogger(String[] args, Logger logger)
    {
    	InitializationData initData = new InitializationData();
	initData.logger = logger;
	return initialize(args, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithProperties(StringSeqHolder args, Properties properties)
    {
    	InitializationData initData = new InitializationData();
	initData.properties = properties;
	return initialize(args, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithProperties(String[] args, Properties properties)
    {
    	InitializationData initData = new InitializationData();
	initData.properties = properties;
	return initialize(args, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithPropertiesAndLogger(StringSeqHolder args, Properties properties, Logger logger)
    {
    	InitializationData initData = new InitializationData();
	initData.properties = properties;
	initData.logger = logger;
	return initialize(args, initData);
    }

    /**
     * @deprecated This method has been deprecated, use initialize instead.
     **/
    public static Communicator
    initializeWithPropertiesAndLogger(String[] args, Properties properties, Logger logger)
    {
    	InitializationData initData = new InitializationData();
	initData.properties = properties;
	initData.logger = logger;
	return initialize(args, initData);
    }

    public static IceInternal.Instance
    getInstance(Communicator communicator)
    {
        CommunicatorI p = (CommunicatorI)communicator;
        return p.getInstance();
    }

    public static IceInternal.ProtocolPluginFacade
    getProtocolPluginFacade(Communicator communicator)
    {
	return new IceInternal.ProtocolPluginFacadeI(communicator);
    }

    public static Identity
    stringToIdentity(String s)
    {
        Identity ident = new Identity();

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
                    IdentityParseException ex = new IdentityParseException();
                    ex.str = s;
                    throw ex;
                }
            }
            pos++;
        }

        if(slash == -1)
        {
            StringHolder token = new StringHolder();
            if(!IceUtil.StringUtil.unescapeString(s, 0, s.length(), token))
            {
                IdentityParseException ex = new IdentityParseException();
                ex.str = s;
                throw ex;
            }
            ident.category = "";
            ident.name = token.value;
        }
        else
        {
            StringHolder token = new StringHolder();
            if(!IceUtil.StringUtil.unescapeString(s, 0, slash, token))
            {
                IdentityParseException ex = new IdentityParseException();
                ex.str = s;
                throw ex;
            }
            ident.category = token.value;
            if(slash + 1 < s.length())
            {
                if(!IceUtil.StringUtil.unescapeString(s, slash + 1, s.length(), token))
                {
                    IdentityParseException ex = new IdentityParseException();
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

    public static String
    identityToString(Identity ident)
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

    public static synchronized String
    generateUUID()
    {
        java.rmi.server.UID uid = new java.rmi.server.UID();

        if(_localAddress == null)
        {
            java.net.InetAddress addr = IceInternal.Network.getLocalAddress();

            byte[] ip = addr.getAddress();
            _localAddress = "";
            for(int i = 0; i < ip.length; i++)
            {
                if(i > 0)
                {
                    _localAddress += ":";
                }
                int n = ip[i] < 0 ? ip[i] + 256 : ip[i];
                _localAddress += Integer.toHexString(n);
            }
        }

        return _localAddress + ":" + uid;
    }

    public static int
    proxyIdentityCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return -1;
	}
	else if(lhs != null && rhs == null)
	{
	    return 1;
	}
	else
	{
	    Identity lhsIdentity = lhs.ice_getIdentity();
	    Identity rhsIdentity = rhs.ice_getIdentity();
	    int n;
	    if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
	    {
		return n;
	    }
	    return lhsIdentity.category.compareTo(rhsIdentity.category);
	}
    }

    public static int
    proxyIdentityAndFacetCompare(ObjectPrx lhs, ObjectPrx rhs)
    {
	if(lhs == null && rhs == null)
	{
	    return 0;
	}
	else if(lhs == null && rhs != null)
	{
	    return -1;
	}
	else if(lhs != null && rhs == null)
	{
	    return 1;
	}
	else
	{
	    Identity lhsIdentity = lhs.ice_getIdentity();
	    Identity rhsIdentity = rhs.ice_getIdentity();
	    int n;
	    if((n = lhsIdentity.name.compareTo(rhsIdentity.name)) != 0)
	    {
		return n;
	    }
	    if((n = lhsIdentity.category.compareTo(rhsIdentity.category)) != 0)
	    {
		return n;
	    }
	    
	    String lhsFacet = lhs.ice_getFacet();
	    String rhsFacet = rhs.ice_getFacet();
            if(lhsFacet == null && rhsFacet == null)
            {
                return 0;
            }
            else if(lhsFacet == null)
            {
                return -1;
            }
            else if(rhsFacet == null)
            {
                return 1;
            }
            return lhsFacet.compareTo(rhsFacet);
	}
    }

    public static InputStream
    createInputStream(Communicator communicator, byte[] bytes)
    {
        return new InputStreamI(communicator, bytes);
    }

    public static OutputStream
    createOutputStream(Communicator communicator)
    {
        return new OutputStreamI(communicator);
    }

    private static Properties _defaultProperties = null;
    private static java.lang.Object _defaultPropertiesMutex = new java.lang.Object();
    private static String _localAddress = null;
}
