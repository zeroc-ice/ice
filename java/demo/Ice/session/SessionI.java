// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionI extends _SessionDisp
{
    public
    SessionI(String name)
    {
        _name = name;
        _timestamp = System.currentTimeMillis();
        System.out.println("The session " + _name + " is now created.");
    }

    synchronized public HelloPrx
    createHello(Ice.Current c)
    {
        if(_destroy)
        {
            throw new Ice.ObjectNotExistException();
        }
        HelloPrx hello = HelloPrxHelper.uncheckedCast(c.adapter.addWithUUID(new HelloI(_name, _nextId++)));
        _objs.add(hello);
        return hello;
    }

    synchronized public void
    refresh(Ice.Current c)
    {
        if(_destroy)
        {
            throw new Ice.ObjectNotExistException();
        }
        _timestamp = System.currentTimeMillis();
    }

    synchronized public String
    getName(Ice.Current c)
    {
        if(_destroy)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _name;
    }
    
    synchronized public void
    destroy(Ice.Current c)
    {
        if(_destroy)
        {
            throw new Ice.ObjectNotExistException();
        }

        _destroy = true;
        System.out.println("The session " + _name + " is now destroyed.");
        try
        {
            c.adapter.remove(c.id);
            java.util.Iterator p = _objs.iterator();
            while(p.hasNext())
            {
                c.adapter.remove(((HelloPrx)p.next()).ice_getIdentity());
            }
        }
        catch(Ice.ObjectAdapterDeactivatedException e)
        {
            // This method is called on shutdown of the server, in
            // which case this exception is expected.
        }
        _objs.clear();
    }

    synchronized public long
    timestamp()
    {
        if(_destroy)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _timestamp;
    }

    private String _name;
    private boolean _destroy = false; // true if destroy() was called, false otherwise.
    private long _timestamp; // The last time the session was refreshed.
    private int _nextId = 0; // The id of the next hello object. This is used for tracing purposes.
    private java.util.List _objs = new java.util.LinkedList(); // List of per-client allocated Hello objects.
}
