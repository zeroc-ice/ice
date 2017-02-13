// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Holder class for floats that are out- or inout-parameters.
 **/
public final class FloatHolder extends Holder<Float>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    FloatHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>float</code> value stored by this holder.
     **/
    public
    FloatHolder(float value)
    {
        super(value);
    }
}
