// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

final class HelloSessionManagerI extends Glacier._SessionManagerDisp
{
    HelloSessionManagerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }

    public Glacier.SessionPrx
    create(String userId, Ice.Current current)
    {
        Glacier.Session session = new HelloSessionI(userId, this);
        Ice.Identity ident = new Ice.Identity();
        ident.category = userId;
        ident.name = "session";

        _adapter.add(session, ident);
        return Glacier.SessionPrxHelper.uncheckedCast(_adapter.createProxy(ident));
    }

    public void
    remove(Ice.Identity ident)
    {
        _adapter.remove(ident);
    }

    private Ice.ObjectAdapter _adapter;
}
