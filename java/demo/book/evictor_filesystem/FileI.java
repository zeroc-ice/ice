// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Filesystem.*;

public final class FileI extends PersistentFile
{
    public
    FileI()
    {
        _destroyed = false;
    }

    @Override
    public synchronized String
    name(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        return nodeName;
    }

    @Override
    public void
    destroy(Ice.Current current)
        throws PermissionDenied
    {
        synchronized(this)
        {
            if(_destroyed)
            {
                throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
            }
            _destroyed = true;
        }

        //
        // Because we use a transactional evictor, these updates are guaranteed to be atomic.
        //
        parent.removeNode(nodeName);
        _evictor.remove(current.id);
    }

    @Override
    public synchronized String[]
    read(Ice.Current current)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        return text.clone();
    }

    @Override
    public synchronized void
    write(String[] text, Ice.Current current)
        throws GenericError
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException(current.id, current.facet, current.operation);
        }

        this.text = text;
    }

    public static Freeze.Evictor _evictor;
    private boolean _destroyed;
}
