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

    /**
     * Returns the Slice type ID of the most-derived interface supported by this object.
     *
     * @return The return value is always <code>::Ice::Object</code>.
     **/
    public String ice_id()
    {
        return ice_staticId();
    }

    public static String ice_staticId()
    {
        return "::Ice::Object";
    }

    public void _iceWrite(OutputStream ostr)
    {
        ostr.startValue(null);
        _iceWriteImpl(ostr);
        ostr.endValue();
    }

    public void _iceRead(InputStream istr)
    {
        istr.startValue();
        _iceReadImpl(istr);
        istr.endValue(false);
    }

    protected void _iceWriteImpl(OutputStream ostr)
    {
    }

    protected void _iceReadImpl(InputStream istr)
    {
    }

    public static final long serialVersionUID = 0L;
}
