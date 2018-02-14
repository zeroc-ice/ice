// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

/**
 * Base class for writing Slice classes to an output stream.
 **/
public abstract class ObjectWriter extends ObjectImpl
{
    /**
     * Writes the state of this Slice class to an output stream.
     *
     * @param out The stream to write to.
     **/
    public abstract void write(OutputStream out);

    @Override
    public void
    __write(IceInternal.BasicStream os)
    {
        OutputStream stream = (OutputStream)os.closure();
        write(stream);
    }

    public void
    __read(IceInternal.BasicStream is, boolean rid)
    {
        assert(false);
    }
}
