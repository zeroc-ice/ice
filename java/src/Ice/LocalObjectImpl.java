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
