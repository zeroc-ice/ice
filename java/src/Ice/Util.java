// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public final class Util
{
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
    getDefaultProperties(String[] args)
    {
        if(_defaultProperties == null)
        {
            _defaultProperties = createProperties(args);
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
    createProperties()
    {
        return new PropertiesI();
    }

    public static Properties
    createProperties(String[] args)
    {
        return new PropertiesI(args);
    }

    public static Properties
    createProperties(StringSeqHolder args)
    {
        return new PropertiesI(args);
    }

    public static Communicator
    initialize(String[] args)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        Properties defaultProperties = getDefaultProperties(argsH);
        CommunicatorI result = new CommunicatorI(argsH, defaultProperties);
        result.finishSetup(argsH);
        return result;
    }

    public static Communicator
    initialize(StringSeqHolder args)
    {
        Properties defaultProperties = getDefaultProperties(args);
        CommunicatorI result = new CommunicatorI(args, defaultProperties);
        result.finishSetup(args);
        return result;
    }

    public static Communicator
    initializeWithProperties(String[] args, Properties properties)
    {
        StringSeqHolder argsH = new StringSeqHolder(args);
        CommunicatorI result = new CommunicatorI(argsH, properties);
        result.finishSetup(argsH);
        return result;
    }

    public static Communicator
    initializeWithProperties(StringSeqHolder args, Properties properties)
    {
        CommunicatorI result = new CommunicatorI(args, properties);
        result.finishSetup(args);
        return result;
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
        int pos = s.indexOf('/');
        if(pos != -1)
        {
            ident.category = s.substring(0, pos);
            ident.name = s.substring(pos + 1);
        }
        else
        {
            ident.category = "";
            ident.name = s;
        }
        return ident;
    }

    public static String
    identityToString(Identity ident)
    {
        if(ident.category.length() == 0)
        {
            return ident.name;
        }
        else
        {
            return ident.category + '/' + ident.name;
        }
    }

    public static synchronized String
    generateUUID()
    {
        java.rmi.server.UID uid = new java.rmi.server.UID();

        if(_localAddress == null)
        {
            java.net.InetAddress addr = null;
            try
            {
                addr = java.net.InetAddress.getLocalHost();
            }
            catch(java.net.UnknownHostException ex)
            {
                throw new DNSException();
            }
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

    private static Properties _defaultProperties = null;
    private static String _localAddress = null;
}
