// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

sealed class HelloSessionManagerI : Glacier._SessionManagerDisp
{
    internal HelloSessionManagerI(Ice.ObjectAdapter adapter)
    {
        _adapter = adapter;
    }
    
    public override Glacier.SessionPrx create(string userId, Ice.Current current)
    {
        Glacier.Session session = new HelloSessionI(userId, this);
        Ice.Identity ident = new Ice.Identity();
        ident.category = userId;
        ident.name = "session";
        
        _adapter.add(session, ident);
        return Glacier.SessionPrxHelper.uncheckedCast(_adapter.createProxy(ident));
    }
    
    public void remove(Ice.Identity ident)
    {
        _adapter.remove(ident);
    }
    
    private Ice.ObjectAdapter _adapter;
}
