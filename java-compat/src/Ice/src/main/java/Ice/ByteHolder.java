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
 * Holder class for bytes that are out- or inout-parameters.
 **/
public final class ByteHolder extends Holder<Byte>
{
    /**
     * Instantiates the class with the value zero.
     **/
    public
    ByteHolder()
    {
    }

    /**
     * Instantiates the class with the passed value.
     *
     * @param value The <code>byte</code> value stored by this holder.
     **/
    public
    ByteHolder(byte value)
    {
        super(value);
    }
}
