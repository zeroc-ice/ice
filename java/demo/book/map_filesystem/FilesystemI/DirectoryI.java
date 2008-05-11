// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        return _dir.name;
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

            NodeDesc[] result = new NodeDesc[_dir.nodes.size()];
            java.util.Iterator<java.util.Map.Entry<String, NodeDesc> > pos = _dir.nodes.entrySet().iterator();
            for(int i = 0; i < _dir.nodes.size(); ++i)
            {
                java.util.Map.Entry<String, NodeDesc> e = pos.next();
                result[i] = e.getValue();
            }
            return result;
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

            NodeDesc nd = _dir.nodes.get(name);
            if(nd == null)
            {
                throw new NoSuchName(name);
            }
            return nd;
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

            if(name.length() == 0 || _dir.nodes.containsKey(name))
            {
                throw new NameInUse(name);
            }

            PersistentFile persistentFile = new PersistentFile();
            persistentFile.name = name;
            Communicator communicator = c.adapter.getCommunicator();
            FileI file = new FileI(c.adapter, communicator.stringToIdentity(Util.generateUUID()),
                                   persistentFile, this);
            assert(_map.get(file.id()) == null);
            _map.put(file.id(), persistentFile);

            FilePrx proxy = FilePrxHelper.uncheckedCast(c.adapter.createProxy(file.id()));

            NodeDesc nd = new NodeDesc();
            nd.name = name;
            nd.type = NodeType.FileType;
            nd.proxy = proxy;
            _dir.nodes.put(name, nd);
            _map.put(_id, _dir);

            file.activate(c.adapter);

            return proxy;
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

            if(name.length() == 0 || _dir.nodes.containsKey(name))
            {
                throw new NameInUse(name);
            }

            PersistentDirectory persistentDir = new PersistentDirectory();
            persistentDir.name = name;
            persistentDir.nodes = new java.util.HashMap<String, NodeDesc>();
            Communicator communicator = c.adapter.getCommunicator();
            DirectoryI dir = new DirectoryI(c.adapter, communicator.stringToIdentity(Util.generateUUID()),
                                            persistentDir, this);
            assert(_map.get(dir.id()) == null);
            _map.put(dir.id(), persistentDir);

            DirectoryPrx proxy = DirectoryPrxHelper.uncheckedCast(c.adapter.createProxy(dir.id()));

            NodeDesc nd = new NodeDesc();
            nd.name = name;
            nd.type = NodeType.DirType;
            nd.proxy = proxy;
            _dir.nodes.put(name, nd);
            _map.put(_id, _dir);

            dir.activate(c.adapter);

            return proxy;
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

                if(_dir.nodes.size() != 0)
                {
                    throw new PermissionDenied("Cannot destroy non-empty directory");
                }

                c.adapter.remove(id());
                _parent.addReapEntry(_dir.name);
                _destroyed = true;
            }
        }
    }

    // DirectoryI constructor. parent == null indicates root directory.

    public DirectoryI(ObjectAdapter adapter, Identity id, PersistentDirectory dir, DirectoryI parent)
    {
        Communicator communicator = adapter.getCommunicator();
        _connection = Freeze.Util.createConnection(communicator, _envName);
        _map = new IdentityNodeMap(_connection, _dbName, false);
        _id = id;
        _dir = dir;
        _parent = parent;
        _destroyed = false;

        java.util.Vector<String> staleEntries = new java.util.Vector<String>();

        java.util.Iterator<java.util.Map.Entry<String, NodeDesc> > p = dir.nodes.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry<String, NodeDesc> e = (java.util.Map.Entry)p.next();
            NodeDesc nd = e.getValue();
            Identity ident = nd.proxy.ice_getIdentity();
            PersistentNode node = (PersistentNode)_map.get(ident);
            NodeI servant;
            if(node != null)
            {
                if(nd.type == NodeType.DirType)
                {
                    PersistentDirectory pDir = (PersistentDirectory)node;
                    servant = new DirectoryI(adapter, ident, pDir, this);
                }
                else
                {
                    PersistentFile pFile = (PersistentFile)node;
                    servant = new FileI(adapter, ident, pFile, this);
                }
                servant.activate(adapter);
            }
            else
            {
                staleEntries.add(e.getKey());
            }
        }

        java.util.Iterator<String> i = staleEntries.iterator();
        while(i.hasNext())
        {
            dir.nodes.remove(i.next());
        }
        if(!staleEntries.isEmpty())
        {
            _map.put(id, dir);
        }
    }


    // Add servant to ASM and to parent's _contents map.

    public DirectoryPrx
    activate(ObjectAdapter a)
    {
        return DirectoryPrxHelper.uncheckedCast(a.add(this, _id));
    }

    // Add this directory and the name of a deleted entry to the reap map.

    public void
    addReapEntry(String name)
    {
        java.util.List<String> l = _reapMap.get(this);
        if(l != null)
        {
            l.add(name);
        }
        else
        {
            l = new java.util.ArrayList<String>();
            l.add(name);
            _reapMap.put(this, l);
        }
    }

    // Remove all names in the reap map from the corresponding directory contents.

    private static void
    reap()
    {
        java.util.Iterator<java.util.Map.Entry<DirectoryI, java.util.List<String> > > i =
            _reapMap.entrySet().iterator();
        while(i.hasNext())
        {
            java.util.Map.Entry<DirectoryI, java.util.List<String> > e = i.next();
            DirectoryI dir = e.getKey();
            java.util.List<String> l = e.getValue();
            java.util.Iterator<String> j = l.iterator();
            while(j.hasNext())
            {
                dir._dir.nodes.remove(j.next());
            }
        }
        _reapMap.clear();
    }

    public static java.lang.Object _lcMutex = new java.lang.Object();

    public static String _envName;
    public static String _dbName;

    private DirectoryI _parent;
    private boolean _destroyed;
    private Identity _id;
    private Freeze.Connection _connection;
    private IdentityNodeMap _map;
    private PersistentDirectory _dir;

    private static java.util.Map<DirectoryI, java.util.List<String> > _reapMap =
        new java.util.HashMap<DirectoryI, java.util.List<String> >();

}
