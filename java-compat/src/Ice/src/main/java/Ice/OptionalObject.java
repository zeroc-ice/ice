// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Handles callbacks for an optional object parameter.
 **/
public class OptionalObject implements ReadValueCallback
{
    /**
     * Instantiates the class with the given optional.
     *
     * @param opt The target optional.
     * @param cls The formal type required for the unmarshaled object.
     * @param type The Slice type ID corresponding to the formal type.
     **/
    @SuppressWarnings("rawtypes")
    public
    OptionalObject(Optional opt, Class<?> cls, String type)
    {
        this.opt = opt;
        this.cls = cls;
        this.type = type;
    }

    /**
     * Sets the value of the optional to the passed instance.
     *
     * @param v The new value for the optional.
     **/
    @SuppressWarnings("unchecked")
    public void
    valueReady(Ice.Object v)
    {
        if(v == null || cls.isInstance(v))
        {
            //
            // The line below would normally cause an "unchecked cast" warning.
            //
            opt.set(v);
        }
        else
        {
            IceInternal.Ex.throwUOE(type, v);
        }
    }

    /**
     * The optional object.
     **/
    @SuppressWarnings("rawtypes")
    public Optional opt;

    /**
     * The formal type of the target class.
     **/
    public Class<?> cls;

    /**
     * The Slice type ID of the target class.
     **/
    public String type;
}
