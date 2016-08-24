// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
     **/
    public
    LocalObjectHolder(java.lang.Object value)
    {
        super(value);
    }
}
