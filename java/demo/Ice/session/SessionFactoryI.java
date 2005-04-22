// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class SessionFactoryI extends _SessionFactoryDisp
{
    public synchronized SessionPrx
    create(String name, Ice.Current c)
    {
	SessionI session = new SessionI(name);
	SessionPrx proxy = SessionPrxHelper.uncheckedCast(c.adapter.addWithUUID(session));
	ReapThread.instance().add(proxy, session);
	return proxy;
    }

    public void
    shutdown(Ice.Current c)
    {
	System.out.println("Shutting down...");
	c.adapter.getCommunicator().shutdown();
    }
}
