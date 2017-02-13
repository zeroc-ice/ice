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
 * For deserialization of Slice types that contain a proxy, the application
 * must instantiate a subclass of <code>ObjectInputStream</code> and supply
 * a communicator that is used to reconstruct the proxy.
 **/
public class ObjectInputStream extends java.io.ObjectInputStream
{
    /**
     * Instantiates this class for the specified communicator and input stream.
     *
     * @param communicator The communicator to use to deserialize proxies.
     * @param stream The input stream to read from.
     * @throws java.io.IOException If an I/O error occurs.
     **/
    public
    ObjectInputStream(Communicator communicator, java.io.InputStream stream)
        throws java.io.IOException
    {
        super(stream);
        _communicator = communicator;
    }

    /**
     * Returns the communicator for this stream.
     * @return The communicator.
     **/
    public Communicator
    getCommunicator()
    {
        return _communicator;
    }

    private Communicator _communicator;
}
