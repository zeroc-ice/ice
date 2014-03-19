// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
    DirectoryI(Ice.Communicator communicator, String name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;

        // Create an identity. The root directory has the fixed identity "RootDir"
        //
        _id = new Ice.Identity();
        _id.name = _parent != null ? java.util.UUID.randomUUID().toString() : "RootDir";
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

    // activate adds the servant to the object adapter and
    // adds child nodes ot the parent's _contents list.

    public void
    activate(Ice.ObjectAdapter a)
    {
        NodePrx thisNode = NodePrxHelper.uncheckedCast(a.add(this, _id));
        if(_parent != null)
        {
            _parent.addChild(thisNode);
        }
    }

    private String _name;
    private DirectoryI _parent;
    private Ice.Identity _id;
    private java.util.List<NodePrx> _contents = new java.util.ArrayList<NodePrx>();
}
