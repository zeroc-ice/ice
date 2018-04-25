// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultServant;

import test.Ice.defaultServant.Test.*;

public final class MyObjectI implements MyObject
{
    @Override
    public void ice_ping(com.zeroc.Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new com.zeroc.Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new com.zeroc.Ice.FacetNotExistException();
        }
    }

    @Override
    public String getName(com.zeroc.Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new com.zeroc.Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new com.zeroc.Ice.FacetNotExistException();
        }

        return name;
    }
}
