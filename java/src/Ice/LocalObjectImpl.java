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

public abstract class LocalObjectImpl implements LocalObject, java.lang.Cloneable
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
        return super.clone();
    }

    public int
    ice_hash()
    {
        return hashCode();
    }
}
