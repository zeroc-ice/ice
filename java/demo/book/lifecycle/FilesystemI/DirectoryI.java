// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package FilesystemI;

import Ice.*;
import Filesystem.*;

public class DirectoryI extends _DirectoryDisp implements NodeI, _DirectoryOperations
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

    public NodeDesc[]
    list(Current c)
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }
        }

        synchronized(_lcMutex)
        {
            reap();

            NodeDesc[] ret = new NodeDesc[_contents.size()];
            java.util.Iterator pos = _contents.entrySet().iterator();
            for(int i = 0; i < _contents.size(); ++i)
            {
                java.util.Map.Entry e = (java.util.Map.Entry)pos.next();
                NodeI p = (NodeI)e.getValue();
                ret[i] = new NodeDesc();
                ret[i].name = (String)e.getKey();
                ret[i].type = p instanceof FileI ? NodeType.FileType : NodeType.DirType;
                ret[i].proxy = NodePrxHelper.uncheckedCast(c.adapter.createProxy(p.id()));
            }
            return ret;
        }
    }

    // Slice find() operation.

    public NodeDesc
    find(String name, Current c) throws NoSuchName
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }
        }

        synchronized(_lcMutex)
        {
            reap();

            NodeI p = (NodeI)_contents.get(name);
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
    }

    // Slice createFile() operation.

    public FilePrx
    createFile(String name, Current c) throws NameInUse
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }
        }

        synchronized(_lcMutex)
        {
            reap();

            if(_contents.containsKey(name))
            {
                throw new NameInUse(name);
            }
            FileI f = new FileI(c.adapter, name, this);
            return FilePrxHelper.uncheckedCast(c.adapter.createProxy(f.id()));
        }
    }

    // Slice createDirectory() operation.

    public DirectoryPrx
    createDirectory(String name, Current c) throws NameInUse
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }
        }

        synchronized(_lcMutex)
        {
            reap();

            if(_contents.containsKey(name))
            {
                throw new NameInUse(name);
            }
            DirectoryI d = new DirectoryI(c.adapter, name, this);
            return DirectoryPrxHelper.uncheckedCast(c.adapter.createProxy(d.id()));
        }
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

            synchronized(_lcMutex)
            {
                reap();

                if(_contents.size() != 0)
                {
                    throw new PermissionDenied("Cannot destroy non-empty directory");
                }

                c.adapter.remove(id());
                _parent.addReapEntry(_name);
                _destroyed = true;
            }
        }
    }

    // DirectoryI constructor for root directory.

    public DirectoryI(ObjectAdapter a)
    {
        this(a, "RootDir", null);
    }

    // DirectoryI constructor. parent == null indicates root directory.

    public DirectoryI(ObjectAdapter a, String name, DirectoryI parent)
    {
        _name = name;
        _parent = parent;
        _id = new Identity();
        _destroyed = false;
        _contents = new java.util.HashMap();

        if(parent == null)
        {
            _id.name = name;
        }
        else
        {
            _id.name = Util.generateUUID();
            _parent.addChild(name, this);
        }
        a.add(this, _id);
    }

    // Add the name-node pair to the _contents map.

    public void
    addChild(String name, NodeI node)
    {
        _contents.put(name, node);
    }

    // Add this directory and the name of a deleted entry to the reap map.

    public void
    addReapEntry(String name)
    {
        java.util.List l = (java.util.List)_reapMap.get(this);
        if(l != null)
        {
            l.add(name);
        }
        else
        {
            java.util.ArrayList v = new java.util.ArrayList();
            v.add(name);
            _reapMap.put(this, v);
        }
    }

    // Remove all names in the reap map from the corresponding directory contents.

    private static void
    reap()
    {
        java.util.Iterator i = _reapMap.entrySet().iterator();
        while(i.hasNext())
        {
            java.util.Map.Entry e = (java.util.Map.Entry)i.next();
            DirectoryI dir = (DirectoryI)e.getKey();
            java.util.List v = (java.util.List)e.getValue();
            java.util.Iterator j = v.iterator();
            while(j.hasNext())
            {
                dir._contents.remove(j.next());
            }
        }
        _reapMap.clear();
    }

    public static java.lang.Object _lcMutex = new java.lang.Object();

    private String _name;       // Immutable
    private DirectoryI _parent; // Immutable
    private Identity _id;       // Immutable
    private boolean _destroyed;
    private java.util.Map _contents;

    private static java.util.Map _reapMap = new java.util.HashMap();
}
