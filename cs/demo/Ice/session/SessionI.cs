// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;
using System;
using System.Collections;

public class SessionI : SessionDisp_
{
    public SessionI(string name)
    {
        _name = name;
        _timestamp = System.DateTime.Now;
        _nextId = 0;
        _destroy = false;
        _objs = new ArrayList();

        Console.Out.WriteLine("The session " + _name + " is now created.");
    }

    public override HelloPrx createHello(Ice.Current c)
    {
        lock(this)
        {
            if(_destroy)
            {
                throw new Ice.ObjectNotExistException();
            }
            
            HelloPrx hello = HelloPrxHelper.uncheckedCast(c.adapter.addWithUUID(new HelloI(_name, _nextId++)));
            _objs.Add(hello);
            return hello;
        }
    }

    public override void refresh(Ice.Current c)
    {
        lock(this)
        {
            if(_destroy)
            {
                throw new Ice.ObjectNotExistException();
            }
            
            _timestamp = System.DateTime.Now;
        }
    }

    public override string getName(Ice.Current c)
    {
        lock(this)
        {
            if(_destroy)
            {
                throw new Ice.ObjectNotExistException();
            }
            
            return _name;
        }
    }

    public override void destroy(Ice.Current c)
    {
        lock(this)
        {
            if(_destroy)
            {
                throw new Ice.ObjectNotExistException();
            }
            
            _destroy = true;
            
            Console.Out.WriteLine("The session " + _name +  " is now destroyed.");
            try
            {
                c.adapter.remove(c.id);
                foreach(HelloPrx p in _objs)
                {
                    c.adapter.remove(p.ice_getIdentity());
                }
            }
            catch(Ice.ObjectAdapterDeactivatedException)
            {
                // This method is called on shutdown of the server, in which
                // case this exception is expected.
            }
        }

        _objs.Clear();
    }

    public System.DateTime timestamp() 
    {
        lock(this)
        {
            if(_destroy)
            {
                throw new Ice.ObjectNotExistException();
            }
            return _timestamp;
        }
    }

    private string _name;
    private System.DateTime _timestamp; // The last time the session was refreshed.
    private int _nextId; // The per-session id of the next hello object. This is used for tracing purposes.
    private ArrayList _objs; // List of per-session allocated hello objects.
    private bool _destroy;
}
