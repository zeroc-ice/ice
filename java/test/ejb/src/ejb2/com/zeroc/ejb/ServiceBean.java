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

@Stateless(name="EJB2ServiceBean")
public class ServiceBean implements Service
{
    @PostConstruct
    public void create()
    {
        com.zeroc.Ice.ObjectPrx db = IceAdapter.stringToProxy("db:tcp -h localhost -p 10002");
        database = DatabasePrx.uncheckedCast(db);
    }

    static class AccountHolder
    {
        Account value;
    }

    public final Account getAccount(String id)
    {
        final AccountHolder holder = new AccountHolder();
        database.getAccountAsync(id).whenComplete((result, ex) ->
            {
                if(ex != null)
                {
                    if(ex instanceof AccountNotExistException)
                    {
                        Account a = new Account(((AccountNotExistException)ex).id, "");
                        database.addAccountAsync(a);
                        synchronized(holder)
                        {
                            holder.value = a;
                            holder.notify();
                        }
                    }
                    else
                    {
                        ex.printStackTrace();
                        assert(false);
                    }
                }
                else
                {
                    synchronized(holder)
                    {
                        holder.value = result;
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

    public final void addAccount(Account s)
    {
        database.addAccount(s);
    }

    private DatabasePrx database;
}
