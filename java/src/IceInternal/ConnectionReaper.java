// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class ConnectionReaper
{
    synchronized public void
    add(Ice.ConnectionI connection)
    {
        _connections.add(connection);
    }

    synchronized public java.util.List<Ice.ConnectionI>
    swapConnections()
    {
        if(_connections.isEmpty())
        {
            return null;
        }
        java.util.List<Ice.ConnectionI> connections = _connections;
        _connections = new java.util.ArrayList<Ice.ConnectionI>();
        return connections;
    }

    private java.util.List<Ice.ConnectionI> _connections = new java.util.ArrayList<Ice.ConnectionI>();
};