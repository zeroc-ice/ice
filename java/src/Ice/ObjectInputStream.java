// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

//
// If you are attempting to deserialize a Slice type that includes a
// proxy, you must instantiate (or subclass) Ice.ObjectInputStream and
// supply a communicator for use in reconstructing the proxy.
//
public class ObjectInputStream extends java.io.ObjectInputStream
{
    public
    ObjectInputStream(Communicator communicator, java.io.InputStream stream)
        throws java.io.IOException
    {
        super(stream);
        _communicator = communicator;
    }

    public Communicator
    getCommunicator()
    {
        return _communicator;
    }

    private Communicator _communicator;
}
