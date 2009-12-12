// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public class FileI extends _FileDisp
{
    public synchronized String
    name(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _file.name;
    }

    public synchronized void
    destroy(Ice.Current c)
        throws PermissionDenied
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(c.id, c.facet, c.operation);
        }
        _destroyed = true;
        _parent.removeEntry(_file.name);
        _map.remove(c.id);
        c.adapter.remove(c.id);
    }

    public synchronized String[]
    read(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(c.id, c.facet, c.operation);
        }
        return _file.text;
    }

    public synchronized void
    write(String[] text, Ice.Current c)
        throws GenericError
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(c.id, c.facet, c.operation);
        }
       _file.text = text;
       _map.put(c.id, _file);
    }

    public
    FileI(PersistentFile file, DirectoryI parent)
    {
        _file = file;
        _parent = parent;
        _destroyed = false;
    }

    public static IdentityNodeMap _map;

    private PersistentFile _file;
    private DirectoryI _parent;
    private boolean _destroyed;
}
