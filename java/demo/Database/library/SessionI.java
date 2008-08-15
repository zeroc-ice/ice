// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionI extends _SessionDisp
{
    public
    SessionI(ConnectionPool pool, Ice.Logger logger, Ice.ObjectAdapter adapter)
    {
        _pool = pool;
        _logger = logger;
        _timestamp = System.currentTimeMillis();
        _libraryImpl = new LibraryI(_logger, _pool);
        _library = LibraryPrxHelper.uncheckedCast(adapter.addWithUUID(_libraryImpl));
    }

    synchronized public LibraryPrx
    getLibrary(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _library;
    }

    synchronized public void
    refresh(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        _timestamp = System.currentTimeMillis();
    }

    synchronized public void
    destroy(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

        _destroyed = true;
        _logger.trace("Session", "The session " + c.id + " is now destroyed.");
        _libraryImpl.destroy();
        try
        {
            if(c != null)
            {
                c.adapter.remove(c.id);
                c.adapter.remove(_library.ice_getIdentity());
            }
        }
        catch(Ice.ObjectAdapterDeactivatedException e)
        {
            // This method is called on shutdown of the server, in
            // which case this exception is expected.
        }
    }

    // Called on application shutdown.
    synchronized public void
    shutdown()
    {
        if(!_destroyed)
        {
            _destroyed = true;
            _libraryImpl.shutdown();
        }
    }

    synchronized public long
    timestamp()
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        return _timestamp;
    }

    private Ice.Logger _logger;
    private ConnectionPool _pool;
    private boolean _destroyed = false; // true if destroy() was called, false otherwise.
    private long _timestamp; // The last time the session was refreshed.
    private LibraryPrx _library;
    private LibraryI _libraryImpl;
}
