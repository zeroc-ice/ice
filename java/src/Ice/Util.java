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

    private static Properties _defaultProperties = null;
}
