// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Filesystem;
using System.Collections;

public class DirectoryI : DirectoryDisp_
{
    // DirectoryI constructor

    public DirectoryI(Ice.Communicator communicator, string name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        //
        // Create an identity. The root directory has the fixed identity "RootDir"
        //
        _id = new Ice.Identity();
        _id.name = _parent != null ? System.Guid.NewGuid().ToString() : "RootDir";
    }

    // Slice Node::name() operation

    public override string name(Ice.Current current)
    {
        return _name;
    }

    // Slice Directory::list() operation

    public override NodePrx[] list(Ice.Current current)
    {
        return (NodePrx[])_contents.ToArray(typeof(NodePrx));
    }

    // addChild is called by the child in order to add
    // itself to the _contents member of the parent

    public void addChild(NodePrx child)
    {
        _contents.Add(child);
    }

    // Add servant to ASM and parent's _contents map.

    public void activate(Ice.ObjectAdapter a)
    {
        NodePrx thisNode = NodePrxHelper.uncheckedCast(a.add(this, _id));
        if(_parent != null)
        {
            _parent.addChild(thisNode);
        }
    }

    private string _name;
    private DirectoryI _parent;
    private Ice.Identity _id;
    private ArrayList _contents = new ArrayList();
}
