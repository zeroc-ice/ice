// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package Ice;

/**
 * Holder class for local objects that are out- or inout-parameters.
 **/
public final class LocalObjectHolder extends Holder<java.lang.Object>
{
    /**
     * Instantiates the class with a <code>null</code> value.
     **/
    public
    LocalObjectHolder()
    {
    }

    /**
     * Instantiates the class with the passed local object.
     *
     * @param value The initial value for this holder.
     **/
    public
    LocalObjectHolder(java.lang.Object value)
    {
        super(value);
    }
}
