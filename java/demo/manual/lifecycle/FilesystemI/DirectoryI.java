// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package FilesystemI;

import Ice.*;
import Filesystem.*;

public class DirectoryI extends _DirectoryDisp implements NodeI
{

    // Slice name() operation.

    public synchronized String
    name(Current c)
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }
        return _name;
    }

    // Return the object identity for this node.

    public Identity
    id()
    {
        return _id;
    }

    // Slice list() operation.

    public synchronized NodeDesc[]
    list(Current c)
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        NodeDesc[] ret = new NodeDesc[_contents.size()];
        java.util.Iterator<java.util.Map.Entry<String, NodeI> > pos = _contents.entrySet().iterator();
        for(int i = 0; i < _contents.size(); ++i)
        {
            java.util.Map.Entry<String, NodeI> e = pos.next();
            NodeI p = e.getValue();
            ret[i] = new NodeDesc();
            ret[i].name = e.getKey();
            ret[i].type = p instanceof FileI ? NodeType.FileType : NodeType.DirType;
            ret[i].proxy = NodePrxHelper.uncheckedCast(c.adapter.createProxy(p.id()));
        }
        return ret;
    }

    // Slice find() operation.

    public synchronized NodeDesc
    find(String name, Current c) throws NoSuchName
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        NodeI p = _contents.get(name);
        if(p == null)
        {
            throw new NoSuchName(name);
        }

        NodeDesc d = new NodeDesc();
        d.name = name;
        d.type = p instanceof FileI ? NodeType.FileType : NodeType.DirType;
        d.proxy = NodePrxHelper.uncheckedCast(c.adapter.createProxy(p.id()));
        return d;
    }

    // Slice createFile() operation.

    public synchronized FilePrx
    createFile(String name, Current c) throws NameInUse
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        if(name.length() == 0 || _contents.containsKey(name))
        {
            throw new NameInUse(name);
        }

        FileI f = new FileI(name, this);
        ObjectPrx node = c.adapter.add(f, f.id());
        _contents.put(name, f);
        return FilePrxHelper.uncheckedCast(node);
    }

    // Slice createDirectory() operation.

    public synchronized DirectoryPrx
    createDirectory(String name, Current c) throws NameInUse
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        if(name.length() == 0 || _contents.containsKey(name))
        {
            throw new NameInUse(name);
        }

        DirectoryI d = new DirectoryI(name, this);
        ObjectPrx node = c.adapter.add(d, d.id());
        _contents.put(name, d);
        return DirectoryPrxHelper.uncheckedCast(node);
    }

    // Slice destroy() operation.

    public void
    destroy(Current c) throws PermissionDenied
    {
        if(_parent == null)
        {
            throw new PermissionDenied("Cannot destroy root directory");
        }

        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }

            if(_contents.size() != 0)
            {
                throw new PermissionDenied("Cannot destroy non-empty directory");
            }

            c.adapter.remove(id());
            _destroyed = true;
        }

        _parent.removeEntry(_name);
    }

    // DirectoryI constructor for root directory.

    public DirectoryI()
    {
        this("/", null);
    }

    // DirectoryI constructor. parent == null indicates root directory.

    public DirectoryI(String name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;
        _id = new Identity();
        _destroyed = false;
        _contents = new java.util.HashMap<String, NodeI>();

        _id.name = parent == null ? "RootDir" : java.util.UUID.randomUUID().toString();
    }

    // Remove the entry from the _contents map.

    public synchronized void
    removeEntry(String name)
    {
        _contents.remove(name);
    }

    private String _name;       // Immutable
    private DirectoryI _parent; // Immutable
    private Identity _id;       // Immutable
    private boolean _destroyed;
    private java.util.Map<String, NodeI> _contents;
}
