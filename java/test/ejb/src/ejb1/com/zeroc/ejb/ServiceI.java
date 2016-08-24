// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.ejb;

import javax.ejb.*;

import com.zeroc.ice.Test.Account;
import com.zeroc.ice.Test.AccountNotExistException;

//
// This Ice servant delegates the calls to the Service EJB
//
public class ServiceI implements com.zeroc.ice.Test.Service
{
    final private Service service;

    public ServiceI(Service service)
    {
        this.service = service;
    }

    public final Account getAccount(String id, com.zeroc.Ice.Current current)
    {
        return service.getAccount(id);
    }

    public final void addAccount(Account s, com.zeroc.Ice.Current current)
    {
        service.addAccount(s);
    }
}
