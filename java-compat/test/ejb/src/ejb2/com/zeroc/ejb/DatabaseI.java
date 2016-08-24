// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import com.zeroc.ice.Test.*;

public class DatabaseI extends _DatabaseDisp
{
    public final Account 
    getAccount(String id, Ice.Current current)
        throws AccountNotExistException
    {
        Account account = accounts.get(id);
        if(account == null)
        {
            throw new AccountNotExistException(id);
        }
        return account;
    }

    public final void
    addAccount(Account s, Ice.Current current)
    {
        accounts.put(s.id, s);
    }

    private java.util.Map<String, Account> accounts = new java.util.HashMap<String, Account>();
}
