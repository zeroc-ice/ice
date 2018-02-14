// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

@Stateless(name="EJB2ServiceBean")
public class ServiceBean implements Service
{
    @PostConstruct
    public void
    create()
    {
        Ice.ObjectPrx db = IceAdapter.stringToProxy("db:tcp -h localhost -p 10002");
        database = DatabasePrxHelper.uncheckedCast(db);
    }

    public final Account
    getAccount(String id)
    {
        final AccountHolder holder = new AccountHolder();
        database.begin_getAccount(id, new Callback_Database_getAccount() {
                public void
                response(Account a)
                {
                    synchronized(holder)
                    {
                        holder.value = a;
                        holder.notify();
                    }
                }

                public void
                exception(Ice.LocalException ex)
                {
                    ex.printStackTrace();
                    assert(false);
                }

                public void
                exception(Ice.UserException ex)
                {
                    Account a = new Account(((AccountNotExistException)ex).id, "");
                    database.begin_addAccount(a);
                    synchronized(holder)
                    {
                        holder.value = a;
                        holder.notify();
                    }
                }
            });

        synchronized(holder)
        {
            while(holder.value == null)
            {
                try
                {
                    holder.wait();
                }
                catch(InterruptedException ex)
                {
                }
            }
        }
        return holder.value;
    }

    public final void
    addAccount(Account s)
    {
        database.addAccount(s);
    }

    private DatabasePrx database;
}
