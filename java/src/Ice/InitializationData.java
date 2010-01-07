// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * A class that encapsulates data to initialize a communicator.
 *
 * @see Communicator#intialize
 * @see Properties
 * @see Logger
 * @see Stats
 * @see ThreadNotification
 **/
public final class InitializationData implements Cloneable
{
    /**
     * Creates an instance with all members set to <code>null</code>.
     **/
    public
    InitializationData()
    {
    }

    /**
     * Creates and returns a copy of this object.
     **/
    public java.lang.Object
    clone()
    {
        //
        // A member-wise copy is safe because the members are immutable.
        //
        java.lang.Object o = null;
        try
        {
            o = super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
        }
        return o;
    }

    /**
     * The properties for the communicator.
     **/
    public Properties properties;

    /**
     * The logger for the communicator.
     **/
    public Logger logger;

    /**
     * The <Stats> instance for the communicator.
     **/
    public Stats stats;

    /**
     * The thread hook for the communicator.
     **/
    public ThreadNotification threadHook;

    /**
     * The custom class loader for the communicator.
     **/
    public ClassLoader classLoader;

    /**
     * The call dispatcher for the communicator.
     **/
    public Dispatcher dispatcher;
}
