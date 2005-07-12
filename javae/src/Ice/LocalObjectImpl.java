// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class LocalObjectImpl implements LocalObject
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
    ice_clone()
        throws IceUtil.CloneException
    {
	try
	{
	    LocalObjectImpl obj = (LocalObjectImpl)getClass().newInstance();
	    obj.__copyFrom(this);
	    return obj;
	}
	catch(java.lang.IllegalAccessException ex)
	{
	    throw new IceUtil.CloneException(ex.getMessage());
	}
	catch(java.lang.InstantiationException ex)
	{
	    throw new IceUtil.CloneException(ex.getMessage());
	}
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

    protected void
    __copyFrom(java.lang.Object obj)
    {
    }
}
