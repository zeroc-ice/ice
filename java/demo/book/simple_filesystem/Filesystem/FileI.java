// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Filesystem;

public class FileI extends _FileDisp
{
    // FileI constructor

    public
    FileI(String name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        assert(_parent != null);

        // Create an identity
        //
        Ice.Identity myID = _adapter.getCommunicator().stringToIdentity(Ice.Util.generateUUID());

        // Add the identity to the object adapter
        //
        _adapter.add(this, myID);

        // Create a proxy for the new node and add it as a child to the parent
        //
        NodePrx thisNode = NodePrxHelper.uncheckedCast(_adapter.createProxy(myID));
        _parent.addChild(thisNode);
    }

    // Slice Node::name() operation

    public String
    name(Ice.Current current)
    {
        return _name;
    }

    // Slice File::read() operation

    public String[]
    read(Ice.Current current)
    {
        return _lines;
    }

    // Slice File::write() operation

    public void
    write(String[] text, Ice.Current current)
        throws GenericError
    {
        _lines = text;
    }

    public static Ice.ObjectAdapter _adapter;
    private String _name;
    private DirectoryI _parent;
    private String[] _lines;
}
