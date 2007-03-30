// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    }

    public
    FileI(Ice.Identity id)
    {
        _ID = id;
    }

    public String
    name(Ice.Current current)
    {
        return nodeName;
    }

    public void
    destroy(Ice.Current current)
        throws PermissionDenied
    {
        parent.removeNode(nodeName);
        _evictor.remove(_ID);
    }

    public synchronized String[]
    read(Ice.Current current)
    {
        return (String[])text.clone();
    }

    public synchronized void
    write(String[] text, Ice.Current current)
        throws GenericError
    {
        this.text = text;
    }

    public static Ice.ObjectAdapter _adapter;
    public static Freeze.Evictor _evictor;
    public Ice.Identity _ID;
}
