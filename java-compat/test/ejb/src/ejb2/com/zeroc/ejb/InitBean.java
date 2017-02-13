// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
@Stateless(mappedName="InitEJB2Service", name="Init2")
public class InitBean implements Init
{
    private @EJB Service service;

    @PostConstruct
    public void 
    create()
    {
        IceAdapter.add(new ServiceI(service), new Ice.Identity("service", "ejb2"));
    }

    @PreDestroy
    public void 
    destroy()
    {
        IceAdapter.remove(new Ice.Identity("service", "ejb2"));
    }

    public void 
    init()
    {
    }
}
