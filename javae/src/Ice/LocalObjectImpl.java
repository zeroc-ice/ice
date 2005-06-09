// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class LocalObjectImpl implements LocalObject, IceUtil.Cloneable
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

    public int
    ice_hash()
    {
        return hashCode();
    }

    protected
    LocalObjectImpl()
    {
    }
    
    protected
    LocalObjectImpl(LocalObjectImpl source)
    {
	//
	// Intentionally left blank.
	//
    }
}
