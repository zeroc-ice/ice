// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
        if(_defaultProperties == null)
        {
            _defaultProperties = createProperties();
        }
        return _defaultProperties;
    }

    public static Properties
    getDefaultProperties(StringSeqHolder args)
    {
        if(_defaultProperties == null)
        {
            _defaultProperties = createProperties(args);
        }
        return _defaultProperties;
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
        Properties properties = getDefaultProperties(args);
	return initializeWithProperties(args, properties);
    }

    public static Communicator
    initialize(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initialize(argsH);
    }

    public static Communicator
    initializeWithProperties(StringSeqHolder args, Properties properties)
    {
        CommunicatorI result = new CommunicatorI(properties);
        result.finishSetup(args);
        return result;
    }

    public static Communicator
    initializeWithProperties(String[] args, Properties properties)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
	return initializeWithProperties(argsH, properties);
    }

    public static IceInternal.Instance
    getInstance(Communicator communicator)
    {
        CommunicatorI p = (CommunicatorI)communicator;
        return p.getInstance();
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
            if(!IceInternal.StringUtil.decodeString(s, 0, s.length(), token))
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
            if(!IceInternal.StringUtil.decodeString(s, 0, slash, token))
            {
                IdentityParseException ex = new IdentityParseException();
                ex.str = s;
                throw ex;
            }
            ident.category = token.value;
            if(slash + 1 < s.length())
            {
                if(!IceInternal.StringUtil.decodeString(s, slash + 1, s.length(), token))
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
            return IceInternal.StringUtil.encodeString(ident.name, "/");
        }
        else
        {
            return IceInternal.StringUtil.encodeString(ident.category, "/") + '/' +
                IceInternal.StringUtil.encodeString(ident.name, "/");
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
	    
	    String[] lhsFacet = lhs.ice_getFacet();
	    String[] rhsFacet = rhs.ice_getFacet();
	    int i;
	    for(i = 0; i < lhsFacet.length && i < rhsFacet.length; i++)
	    {
		if((n = lhsFacet[i].compareTo(rhsFacet[i])) != 0)
		{
		    return n;
		}
	    }
	    
	    if(lhsFacet.length == rhsFacet.length)
	    {
		return 0;
	    }
	    else if(lhsFacet.length < rhsFacet.length)
	    {
		return -1;
	    }
	    else
	    {
		return 1;
	    }
	}
    }

    private static Properties _defaultProperties = null;
    private static String _localAddress = null;
}
