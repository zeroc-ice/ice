// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package Freeze;

public final class EvictionStrategyI extends Ice.LocalObjectImpl implements EvictionStrategy
{
    public
    EvictionStrategyI()
    {
    }

    public Ice.LocalObject
    activatedObject(Ice.Identity ident, Ice.Object servant)
    {
        return new Cookie();
    }

    public void
    destroyedObject(Ice.Identity ident, Ice.LocalObject cookie)
    {
        // Nothing to do
    }

    public void
    evictedObject(ObjectStore store, Ice.Identity ident, Ice.Object servant, Ice.LocalObject cookie)
    {
        //
        // Only store the object's persistent state if it has been mutated.
        //
        Cookie c = (Cookie)cookie;
        if(c.mutated)
        {
            store.save(ident, servant);
            c.mutated = false;
        }
    }

    public void
    invokedObject(ObjectStore store, Ice.Identity ident, Ice.Object servant, boolean mutating, boolean idle,
                  Ice.LocalObject cookie)
    {
        if(mutating)
        {
            Cookie c = (Cookie)cookie;
            c.mutated = true;
        }
    }

    public void
    destroy()
    {
        // Nothing to do
    }

    private static class Cookie extends Ice.LocalObjectImpl
    {
        boolean mutated = false;
    }
}
