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

public final class IdleStrategyI extends Ice.LocalObjectImpl implements IdleStrategy
{
    public
    IdleStrategyI()
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
        // The object must reach the idle state in order for it to be
        // evicted, therefore the object should have already been saved
        // by invokedObject.
        //
        Cookie c = (Cookie)cookie;
        assert(!c.mutated);
    }

    public void
    invokedObject(ObjectStore store, Ice.Identity ident, Ice.Object servant, boolean mutating, boolean idle,
                  Ice.LocalObject cookie)
    {
        Cookie c = (Cookie)cookie;
        if(!idle && mutating)
        {
            //
            // Mark the object as mutated so we can save it when idle.
            //
            c.mutated = true;
        }
        else if(idle && (mutating || c.mutated))
        {
            //
            // Only store the object's persistent state if the object is idle
            // and it has been mutated.
            //
            store.save(ident, servant);
            c.mutated = false;
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
