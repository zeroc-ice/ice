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
import FilesystemI.*;

public class FileI extends _FileDisp implements NodeI, _FileOperations
{
    public synchronized String
    name(Current c)
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }
        return _file.name;
    }

    public Identity
    id()
    {
        return _id;
    }

    public synchronized String[]
    read(Current c)
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        return _file.text;
    }

    public synchronized void
    write(String[] text, Current c)
    {
        if(_destroyed)
        {
            throw new ObjectNotExistException();
        }

        _file.text = (String[])text.clone();
        _map.put(_id, _file);
    }

    public void
    destroy(Current c)
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new ObjectNotExistException();
            }
            _destroyed = true;
        }

        synchronized(_parent._lcMutex)
        {
            c.adapter.remove(c.id);
            _parent.addReapEntry(_file.name);
            _map.remove(c.id);
        }
    }

    public FileI(ObjectAdapter adapter, Ice.Identity id, PersistentFile file, DirectoryI parent)
    {
        _connection = Freeze.Util.createConnection(adapter.getCommunicator(), _envName);
        _map = new IdentityNodeMap(_connection, _dbName, false);
        _id = id;
        _file = file;
        _parent = parent;
        _destroyed = false;
    }

    public FilePrx
    activate(Ice.ObjectAdapter a)
    {
        return FilePrxHelper.uncheckedCast(a.add(this, _id));
    }

    public static String _envName;
    public static String _dbName;

    private DirectoryI _parent;
    private boolean _destroyed;
    private Identity _id;
    private Freeze.Connection _connection;
    private IdentityNodeMap _map;
    private PersistentFile _file;
}
