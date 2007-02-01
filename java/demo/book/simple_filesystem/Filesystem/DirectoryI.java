// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Filesystem;

public final class DirectoryI extends _DirectoryDisp
{
    // DirectoryI constructor

    public
    DirectoryI(String name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        // Create an identity. The parent has the fixed identity "/"
        //
        Ice.Identity myID = 
            _adapter.getCommunicator().stringToIdentity(_parent != null ? Ice.Util.generateUUID() : "RootDir");

        // Add the identity to the object adapter
        //
        _adapter.add(this, myID);

        // Create a proxy for the new node and add it as a child to the parent
        //
        NodePrx thisNode = NodePrxHelper.uncheckedCast(_adapter.createProxy(myID));
        if (_parent != null)
            _parent.addChild(thisNode);
    }

    // Slice Node::name() operation

    public String
    name(Ice.Current current)
    {
        return _name;
    }

    // Slice Directory::list() operation

    public NodePrx[]
    list(Ice.Current current)
    {
        NodePrx[] result = new NodePrx[_contents.size()];
        _contents.toArray(result);
        return result;
    }

    // addChild is called by the child in order to add
    // itself to the _contents member of the parent

    void
    addChild(NodePrx child)
    {
        _contents.add(child);
    }

    public static Ice.ObjectAdapter _adapter;
    private String _name;
    private DirectoryI _parent;
    private java.util.ArrayList _contents = new java.util.ArrayList();
}
