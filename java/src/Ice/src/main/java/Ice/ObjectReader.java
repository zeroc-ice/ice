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
 * Base class for extracting objects from an input stream.
 **/
public abstract class ObjectReader extends ObjectImpl
{
    /**
     * Reads the state of this Slice class from an input stream.
     *
     * @param in The input stream to read from.
     **/
    public abstract void read(InputStream in);

    @Override
    public void
    __write(IceInternal.BasicStream os)
    {
        assert(false);
    }

    @Override
    public void
    __read(IceInternal.BasicStream is)
    {
        InputStream stream = (InputStream)is.closure();
        read(stream);
    }
}
