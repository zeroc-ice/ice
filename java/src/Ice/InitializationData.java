// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public final class InitializationData implements Cloneable
{
    public
    InitializationData()
    {
    }

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

    public Properties properties;
    public Logger logger;
    public Stats stats;
    public ThreadNotification threadHook;
}
