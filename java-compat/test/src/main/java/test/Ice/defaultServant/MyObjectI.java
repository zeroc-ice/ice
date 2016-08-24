// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.defaultServant;
import test.Ice.defaultServant.Test.*;

public final class MyObjectI extends _MyObjectDisp
{
    @Override
    public void
    ice_ping(Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new Ice.FacetNotExistException();
        }
    }

    @Override
    public String
    getName(Ice.Current current)
    {
        String name = current.id.name;

        if(name.equals("ObjectNotExist"))
        {
            throw new Ice.ObjectNotExistException();
        }
        else if(name.equals("FacetNotExist"))
        {
            throw new Ice.FacetNotExistException();
        }

        return name;
    }
}
