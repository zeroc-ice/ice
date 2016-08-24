// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * The base class for instances of Slice classes.
 **/
public abstract class Value implements java.lang.Cloneable, java.io.Serializable
{
    /**
     * Returns a copy of the object. The cloned object contains field-for-field copies
     * of the state.
     *
     * @return The cloned object.
     **/
    public Value clone()
    {
        Value c = null;

        try
        {
            c = (Value)super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert(false);
        }

        return c;
    }

    /**
     * The Ice run time invokes this method prior to marshaling an object's data members. This allows a subclass
     * to override this method in order to validate its data members.
     **/
    public void ice_preMarshal()
    {
    }

    /**
     * The Ice run time invokes this method vafter unmarshaling an object's data members. This allows a
     * subclass to override this method in order to perform additional initialization.
     **/
    public void ice_postUnmarshal()
    {
    }

    public static final String ice_staticId = "::Ice::Object";

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @return The return value is always <code>::Ice::Object</code>.
     **/
    public String ice_id()
    {
        return ice_staticId;
    }

    public void __write(OutputStream __os)
    {
        __os.startValue(null);
        __writeImpl(__os);
        __os.endValue();
    }

    public void __read(InputStream __is)
    {
        __is.startValue();
        __readImpl(__is);
        __is.endValue(false);
    }

    protected void __writeImpl(OutputStream __os)
    {
    }

    protected void __readImpl(InputStream __is)
    {
    }

    public static final long serialVersionUID = 0L;
}
