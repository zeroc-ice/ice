// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
        // TODO: Fix this when interfaces extend Ice.Object
        //Glacier.Session session = new HelloSessionI(userId, this);
        HelloSessionI session = new HelloSessionI(userId, this);
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
