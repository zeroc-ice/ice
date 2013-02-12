// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionI implements _SessionOperations, _Glacier2SessionOperations
{
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

    synchronized public long
    getSessionTimeout(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }
        return 5000;
    }

    synchronized public void
    destroy(Ice.Current c)
    {
        if(_destroyed)
        {
            throw new Ice.ObjectNotExistException();
        }

        _destroyed = true;
        _logger.trace("Session", "session " + c.adapter.getCommunicator().identityToString(c.id) +
                      " is now destroyed.");

        // This method is never called on shutdown of the server.
        _libraryI.destroy();
        c.adapter.remove(_library.ice_getIdentity());
        c.adapter.remove(c.id);
    }

    // Called on application shutdown.
    synchronized public void
    shutdown()
    {
        if(!_destroyed)
        {
            _destroyed = true;
            _libraryI.shutdown();
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

    SessionI(Ice.Logger logger, Ice.ObjectAdapter adapter)
    {
        _logger = logger;
        _timestamp = System.currentTimeMillis();
        _libraryI = new LibraryI();
        _library = LibraryPrxHelper.uncheckedCast(adapter.addWithUUID(new DispatchInterceptorI(_libraryI)));
    }

    private Ice.Logger _logger;
    private boolean _destroyed = false; // true if destroy() was called, false otherwise.
    private long _timestamp; // The last time the session was refreshed.
    private LibraryPrx _library;
    private LibraryI _libraryI;
}
