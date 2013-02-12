// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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

    public FileI(Ice.Communicator communicator, string name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        Debug.Assert(_parent != null);

        //
        // Create an identity
        //
        _id = new Ice.Identity();
        _id.name = System.Guid.NewGuid().ToString();
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

    // Add servant to ASM and parent's _contents map.

    public void activate(Ice.ObjectAdapter a)
    {
        NodePrx thisNode = NodePrxHelper.uncheckedCast(a.add(this, _id));
        _parent.addChild(thisNode);
    }

    private string _name;
    private DirectoryI _parent;
    private Ice.Identity _id;
    private string[] _lines;
}
