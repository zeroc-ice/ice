// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
