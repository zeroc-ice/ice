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

public abstract class LocalObject
{
    public boolean
    equals(java.lang.Object rhs)
    {
        try
        {
            LocalObject r = (LocalObject)rhs;
            return this == r;
        }
        catch(ClassCastException ex)
        {
        }
        return false;
    }

    public java.lang.Object
    clone()
        throws java.lang.CloneNotSupportedException
    {
        LocalObject result = null;

        try
        {
            result = (LocalObject)getClass().newInstance();
            result.ice_copyStateFrom(this);
        }
        catch(InstantiationException ex)
        {
            CloneNotSupportedException e = new CloneNotSupportedException();
            e.initCause(ex);
            throw e;
        }
        catch(IllegalAccessException ex)
        {
            CloneNotSupportedException e = new CloneNotSupportedException();
            e.initCause(ex);
            throw e;
        }

        return result;
    }

    protected void
    ice_copyStateFrom(LocalObject obj)
        throws java.lang.CloneNotSupportedException
    {
    }

    public int
    ice_hash()
    {
        return hashCode();
    }
}
