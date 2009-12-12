// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class DirectoryI extends _DirectoryDisp
{
    public synchronized void
    destroy(Ice.Current c)
        throws PermissionDenied
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(c.id, c.facet, c.operation);
        }
        if(_parent == null)
        {
            throw new PermissionDenied("Cannot destroy root directory");
        }
        if(!_dir.nodes.isEmpty())
        {
            throw new PermissionDenied("Cannot destroy non-empty directory");
        }
        _destroyed = true;
        _parent.removeEntry(_dir.name);
        _map.remove(c.id);
        c.adapter.remove(c.id);
    }

    public synchronized String
    name(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _dir.name;
    }

    public synchronized NodeDesc[]
    list(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        NodeDesc[] result = new NodeDesc[_dir.nodes.size()];
        java.util.Iterator<NodeDesc> p = _dir.nodes.values().iterator();
        for(int i = 0; i < _dir.nodes.size(); ++i)
        {
            result[i] = p.next();
        }
        return result;
    }

    public synchronized NodeDesc
    find(String name, Ice.Current c)
        throws NoSuchName
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        NodeDesc nd = _dir.nodes.get(name);
        if(nd == null)
        {
            throw new NoSuchName(name);
        }
        return nd;
    }

    public synchronized FilePrx
    createFile(String name, Ice.Current c)
        throws NameInUse
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

        if(name.isEmpty() || _dir.nodes.get(name) != null)
        {
            throw new NameInUse(name);
        }

        PersistentFile persistentFile = new PersistentFile();
        persistentFile.name = name;
        Ice.Identity id = _communicator.stringToIdentity(java.util.UUID.randomUUID().toString());
        FileI file = new FileI(persistentFile, this);
        _map.put(id, persistentFile);

        FilePrx proxy = FilePrxHelper.uncheckedCast(c.adapter.createProxy(id));

        NodeDesc nd = new NodeDesc();
        nd.name = name;
        nd.type = NodeType.FileType;
        nd.proxy = proxy;
        _dir.nodes.put(name, nd);

        _map.put(c.id, _dir);

        _adapter.add(file, id);

        return proxy;
    }

    public synchronized DirectoryPrx
    createDirectory(String name, Ice.Current c)
        throws NameInUse
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

        if(name.isEmpty() || _dir.nodes.get(name) != null)
        {
            throw new NameInUse(name);
        }

        PersistentDirectory persistentDir = new PersistentDirectory();
        persistentDir.name = name;
        persistentDir.nodes = new java.util.HashMap<String, NodeDesc>();
        Ice.Identity id = _communicator.stringToIdentity(java.util.UUID.randomUUID().toString());
        DirectoryI dir = new DirectoryI(id, persistentDir, this);
        _map.put(id, persistentDir);

        DirectoryPrx proxy = DirectoryPrxHelper.uncheckedCast(c.adapter.createProxy(id));

        NodeDesc nd = new NodeDesc();
        nd.name = name;
        nd.type = NodeType.DirType;
        nd.proxy = proxy;
        _dir.nodes.put(name, nd);

        _map.put(c.id, _dir);

        _adapter.add(dir, id);

        return proxy;
    }

    //
    // Called by the child to remove itself from the parent's node map when the child is destroyed.
    //
    public synchronized void
    removeEntry(String name)
    {
        _dir.nodes.remove(name);
        _map.put(_id, _dir);
    }

    public
    DirectoryI(Ice.Identity pid, PersistentDirectory dir, DirectoryI parent)
    {
        _id = pid;
        _dir = dir;
        _parent = parent;
        _destroyed = false;

        // Instantiate the child nodes
        //
        java.util.Iterator<NodeDesc> p = _dir.nodes.values().iterator();
        while(p.hasNext())
        {
            NodeDesc desc = p.next();
            Ice.Identity id = desc.proxy.ice_getIdentity();
            PersistentNode node = _map.get(id);
            assert(node != null);
            if(desc.type == NodeType.DirType)
            {
                PersistentDirectory pDir = (PersistentDirectory)node;
                assert(pDir != null);
                DirectoryI d = new DirectoryI(id, pDir, this);
                _adapter.add(d, id);
            }
            else
            {
                PersistentFile pFile = (PersistentFile)node;
                assert(pFile != null);
                FileI f = new FileI(pFile, this);
                _adapter.add(f, id);
            }
        }
    }

    public static Ice.Communicator _communicator;
    public static Ice.ObjectAdapter _adapter;
    public static IdentityNodeMap _map;

    private Ice.Identity _id;
    private PersistentDirectory _dir;
    private DirectoryI _parent;
    private boolean _destroyed;
}
