// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public final class DirectoryI extends PersistentDirectory
{
    public
    DirectoryI()
    {
        _destroyed = false;
    }

    public
    DirectoryI(Ice.Identity id)
    {
        _ID = id;
        nodes = new java.util.HashMap();
        _destroyed = false;
    }

    public synchronized String
    name(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        return nodeName;
    }

    public void
    destroy(Ice.Current current)
        throws PermissionDenied
    {
        if(parent == null)
        {
            throw new PermissionDenied("cannot remove root directory");
        }

        java.util.Map children = null;

        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
            }

            children = (java.util.Map)((java.util.HashMap)nodes).clone();
            _destroyed = true;
        }

        //
        // For consistency with C++, we iterate over the children outside of synchronization.
        //
        java.util.Iterator p = children.values().iterator();
        while(p.hasNext())
        {
            NodeDesc desc = (NodeDesc)p.next();
            desc.proxy.destroy();
        }

        assert(nodes.isEmpty());

        parent.removeNode(nodeName);
        _evictor.remove(_ID);
    }

    public java.util.Map
    list(ListMode mode, Ice.Current current)
    {
        java.util.Map result = null;
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
            }

            result = (java.util.Map)((java.util.HashMap)nodes).clone();
        }

        if(mode == ListMode.RecursiveList)
        {
            java.util.Map children = new java.util.HashMap();
            java.util.Iterator p = result.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry e = (java.util.Map.Entry)p.next();
                NodeDesc desc = (NodeDesc)e.getValue();
                if(desc.type == NodeType.DirType)
                {
                    DirectoryPrx dir = DirectoryPrxHelper.uncheckedCast(desc.proxy);
                    try
                    {
                        java.util.Map d = dir.list(mode);
                        java.util.Iterator q = d.entrySet().iterator();
                        while(q.hasNext())
                        {
                            java.util.Map.Entry e2 = (java.util.Map.Entry)q.next();
                            NodeDesc desc2 = (NodeDesc)e2.getValue();
                            children.put(desc.name + "/" + desc2.name, desc2);
                        }
                    }
                    catch(Ice.ObjectNotExistException ex)
                    {
                        // This node may have been destroyed, so skip it.
                    }
                }
            }
            result.putAll(children);
        }

        return result;
    }

    public NodeDesc
    resolve(String path, Ice.Current current)
        throws NoSuchName
    {
        int pos = path.indexOf('/');
        String child, remainder = null;
        if(pos == -1)
        {
            child = path;
        }
        else
        {
            child = path.substring(0, pos);
            while(pos < path.length() && path.charAt(pos) == '/')
            {
                ++pos;
            }
            if(pos < path.length())
            {
                remainder = path.substring(pos);
            }
        }

        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
            }

            if(!nodes.containsKey(child))
            {
                throw new NoSuchName("no node exists with name `" + child + "'");
            }

            NodeDesc desc = (NodeDesc)nodes.get(child);
            if(remainder == null)
            {
                return desc;
            }
            else
            {
                if(desc.type != NodeType.DirType)
                {
                    throw new NoSuchName("node `" + child + "' is not a directory");
                }
                DirectoryPrx dir = DirectoryPrxHelper.checkedCast(desc.proxy);
                assert(dir != null);
                return dir.resolve(remainder);
            }
        }
    }

    public synchronized DirectoryPrx
    createDirectory(String name, Ice.Current current)
        throws IllegalName,
               NameInUse
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        checkName(name);

        Ice.Identity id = current.adapter.getCommunicator().stringToIdentity(Ice.Util.generateUUID());
        PersistentDirectory dir = new DirectoryI(id);
        dir.nodeName = name;
        dir.parent = PersistentDirectoryPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
        DirectoryPrx proxy = DirectoryPrxHelper.uncheckedCast(_evictor.add(dir, id));

        NodeDesc nd = new NodeDesc();
        nd.name = name;
        nd.type = NodeType.DirType;
        nd.proxy = proxy;
        nodes.put(name, nd);

        return proxy;
    }

    public synchronized FilePrx
    createFile(String name, Ice.Current current)
        throws IllegalName,
               NameInUse
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        checkName(name);

        Ice.Identity id = current.adapter.getCommunicator().stringToIdentity(Ice.Util.generateUUID());
        PersistentFile file = new FileI(id);
        file.nodeName = name;
        file.parent = PersistentDirectoryPrxHelper.uncheckedCast(current.adapter.createProxy(current.id));
        FilePrx proxy = FilePrxHelper.uncheckedCast(_evictor.add(file, id));

        NodeDesc nd = new NodeDesc();
        nd.name = name;
        nd.type = NodeType.FileType;
        nd.proxy = proxy;
        nodes.put(name, nd);

        return proxy;
    }

    public synchronized void
    removeNode(String name, Ice.Current current)
    {
        assert(nodes.containsKey(name));
        nodes.remove(name);
    }

    private
    void checkName(String name)
        throws IllegalName, NameInUse
    {
        if(name.length() == 0 || name.indexOf('/') >= 0)
        {
            throw new IllegalName("illegal name `" + name + "'");
        }

        if(nodes.containsKey(name))
        {
            throw new NameInUse("name `" + name + "' is already in use");
        }
    }

    public static Ice.ObjectAdapter _adapter;
    public static Freeze.Evictor _evictor;
    public Ice.Identity _ID;
    private boolean _destroyed;
}
