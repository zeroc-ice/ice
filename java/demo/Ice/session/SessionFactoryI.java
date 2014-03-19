// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionFactoryI extends _SessionFactoryDisp
{
    SessionFactoryI(ReapThread reaper)
    {
        _reaper = reaper;
    }

    public synchronized SessionPrx
    create(String name, Ice.Current c)
    {
        SessionI session = new SessionI(name);
        SessionPrx proxy = SessionPrxHelper.uncheckedCast(c.adapter.addWithUUID(session));
        _reaper.add(proxy, session);
        return proxy;
    }

    public void
    shutdown(Ice.Current c)
    {
        System.out.println("Shutting down...");
        c.adapter.getCommunicator().shutdown();
    }

    private ReapThread _reaper;
}
