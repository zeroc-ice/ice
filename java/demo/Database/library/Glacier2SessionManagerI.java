// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class Glacier2SessionManagerI extends Glacier2._SessionManagerDisp
{
    public Glacier2.SessionPrx
    create(String userId, Glacier2.SessionControlPrx control, Ice.Current c)
    {
        SessionI session = new SessionI(_logger, _pool, c.adapter, _libraryServant);
        _Glacier2SessionTie servant = new _Glacier2SessionTie(session);
        Glacier2.SessionPrx proxy = Glacier2.SessionPrxHelper.uncheckedCast(c.adapter.addWithUUID(servant));
        _logger.trace("SessionFactory", "create new session: " +
                      c.adapter.getCommunicator().identityToString(proxy.ice_getIdentity()));
        _reaper.add(proxy, session);
        return proxy;
    }

    Glacier2SessionManagerI(Ice.Logger logger, ReapThread reaper, ConnectionPool pool, Ice.Object libraryServant)
    {
        _logger = logger;
        _reaper = reaper;
        _pool = pool;
        _libraryServant = libraryServant;
    }

    private Ice.Logger _logger;
    private ReapThread _reaper;
    private ConnectionPool _pool;
    private Ice.Object _libraryServant;
}
