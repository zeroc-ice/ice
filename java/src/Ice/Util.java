// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public final class Util
{
    public static Properties
    getDefaultProperties(String[] args)
    {
        if (_defaultProperties == null)
        {
            _defaultProperties = createProperties(args);
        }
        return _defaultProperties;
    }

    public static Properties
    createProperties(String[] args)
    {
        return new PropertiesI(args);
    }

    public static Properties
    createPropertiesFromFile(String[] args, String file)
    {
        return new PropertiesI(args, file);
    }

    public static Communicator
    initialize(String[] args)
    {
        return new CommunicatorI(getDefaultProperties(args));
    }

    public static Communicator
    initializeWithProperties(Properties properties)
    {
        return new CommunicatorI(properties);
    }

    public static void
    addArgumentPrefix(String prefix)
    {
        PropertiesI.addArgumentPrefix(prefix);
    }

    public static Identity
    stringToIdentity(String s)
    {
        Identity ident = new Identity();
        int pos = s.indexOf('#');
        if (pos != -1)
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
        if (ident.category.length() == 0)
        {
            return ident.name;
        }
        else
        {
            return ident.category + '#' + ident.name;
        }
    }

    private static Properties _defaultProperties = null;
}
