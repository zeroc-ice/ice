// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
     * @param rid If <code>true</code>, extraction begins by reading a Slice type ID
     * first. If <code>false</code>, the leading type ID is not read. This is used
     * by the unmarshaling code in case the type ID has already been read as part
     * of other unmarshaling activities.
     **/
    public abstract void read(InputStream in);

    public void
    __write(IceInternal.BasicStream os)
    {
        assert(false);
    }

    public void
    __read(IceInternal.BasicStream is)
    {
        InputStream stream = (InputStream)is.closure();
        read(stream);
    }
}
