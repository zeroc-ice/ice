// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.io.IOException;
import java.io.InputStream;

/**
 * For deserialization of Slice types that contain a proxy, applications must instantiate a subclass of
 * {@code ObjectInputStream} and supply a communicator that is used to construct the proxy.
 */
public class ObjectInputStream extends java.io.ObjectInputStream {
    /**
     * Creates an ObjectInputStream with the specified communicator and input stream.
     *
     * @param communicator the communicator to use to deserialize proxies
     * @param stream the input stream to read from
     * @throws IOException if an I/O error occurs
     */
    public ObjectInputStream(Communicator communicator, InputStream stream) throws IOException {
        super(stream);
        _communicator = communicator;
    }

    /**
     * Returns the communicator used by this stream.
     *
     * @return the communicator
     */
    public Communicator getCommunicator() {
        return _communicator;
    }

    private final Communicator _communicator;
}
