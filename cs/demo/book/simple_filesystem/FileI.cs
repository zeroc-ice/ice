// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Filesystem;
using System.Diagnostics;

public class FileI : FileDisp_
{
    // FileI constructor

    public FileI(string name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        Debug.Assert(_parent != null);

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

    public override string name(Ice.Current current)
    {
        return _name;
    }

    // Slice File::read() operation

    public override string[] read(Ice.Current current)
    {
        return _lines;
    }

    // Slice File::write() operation

    public override void write(string[] text, Ice.Current current)
    {
        _lines = text;
    }

    public static Ice.ObjectAdapter _adapter;
    private string _name;
    private DirectoryI _parent;
    private string[] _lines;
}
