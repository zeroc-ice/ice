// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import javax.annotation.*;
import javax.ejb.*;
import com.zeroc.ice.Test.Account;

@Stateless(name="EJB1ServiceBean")
public class ServiceBean implements Service
{
    public final Account 
    getAccount()
    {
        return this.account;
    }

    public final void
    setAccount(Account s)
    {
        this.account = s;
    }

    private Account account;
}
