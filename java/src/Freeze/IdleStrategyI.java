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
        // by postOperation.
        //
        Cookie c = (Cookie)cookie;
        assert(!c.mutated);
        assert(c.mutatingCount == 0);
    }

    public void
    savedObject(ObjectStore store, Ice.Identity ident, Ice.Object servant, Ice.LocalObject cookie, int usageCount)
    {
	assert(usageCount > 0);

	if(usageCount == 1)
	{
	    Cookie c = (Cookie)cookie;
	    c.mutated = false;
	}    
    }

    public void
    preOperation(ObjectStore store, Ice.Identity ident, Ice.Object servant, boolean mutating, Ice.LocalObject cookie)
    {
        Cookie c = (Cookie)cookie;
        if(mutating)
        {
            ++c.mutatingCount;
            c.mutated = true;
        }
        else if(c.mutatingCount == 0 && c.mutated)
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
    postOperation(ObjectStore store, Ice.Identity ident, Ice.Object servant, boolean mutating, Ice.LocalObject cookie)
    {
        Cookie c = (Cookie)cookie;
        if(mutating)
        {
            assert(c.mutatingCount >= 1);
            --c.mutatingCount;
        }
        if(c.mutatingCount == 0 && c.mutated)
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
        int mutatingCount = 0;
        boolean mutated = false;
    }
}
