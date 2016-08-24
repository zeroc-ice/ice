// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import javax.annotation.*;
import javax.ejb.*;
import com.zeroc.ice.IceAdapter;
import com.zeroc.ice.Test.*;

@Stateless(name="EJB1ServiceBean")
public class ServiceBean implements Service
{
    @PostConstruct
    public void 
    create()
    {
        Ice.ObjectPrx db = IceAdapter.stringToProxy("db:tcp -h localhost -p 10001");
        database = DatabasePrxHelper.uncheckedCast(db);
    }

    public final Account 
    getAccount(String id)
    {
        try
        {
            return database.getAccount(id);
        }
        catch(AccountNotExistException ex)
        {
            Account a = new Account(id);
            database.addAccount(a);
            return a;
        }
    }

    public final void
    addAccount(Account s)
    {
        database.addAccount(s);
    }

    private DatabasePrx database;
}
