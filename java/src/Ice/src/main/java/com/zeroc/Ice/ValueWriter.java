// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.Ice;

/**
 * Base class for writing a Slice class instance to an output stream.
 **/
public abstract class ValueWriter extends Value
{
    /**
     * Writes the state of this Slice class to an output stream.
     *
     * @param out The stream to write to.
     **/
    public abstract void write(OutputStream out);

    @Override
    public void _iceWrite(OutputStream os)
    {
        write(os);
    }

    @Override
    public void _iceRead(InputStream is)
    {
        assert(false);
    }

    public static final long serialVersionUID = 0L;
}
