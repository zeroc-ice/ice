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

//
// The initialization EJB is used to register Ice servants for each
// the EJBs from this EJB3 Jar.
//
@Stateless(mappedName="InitEJB1Service", name="Init1")
public class InitBean implements Init
{
    private @EJB Service service;

    @PostConstruct
    public void create()
    {
        IceAdapter.add(new ServiceI(service), new com.zeroc.Ice.Identity("service", "ejb1"));
    }

    @PreDestroy
    public void destroy()
    {
        IceAdapter.remove(new com.zeroc.Ice.Identity("service", "ejb1"));
    }

    public void 
    init()
    {
    }
}
