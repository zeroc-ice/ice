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

public final class StrategyI extends Ice.LocalObjectImpl implements Freeze.PersistenceStrategy
{
    public
    StrategyI(Freeze.PersistenceStrategy delegate)
    {
        _delegate = delegate;
    }

    public Ice.LocalObject
    activatedObject(Ice.Identity ident, Ice.Object servant)
    {
        return _delegate.activatedObject(ident, servant);
    }

    public void
    destroyedObject(Ice.Identity ident, Ice.LocalObject cookie)
    {
        _delegate.destroyedObject(ident, cookie);
    }

    public void
    evictedObject(Freeze.ObjectStore store, Ice.Identity ident, Ice.Object servant, Ice.LocalObject cookie)
    {
        Test.Servant s = (Test.Servant)servant;
        _lastEvictedValue = s.getValue(null);

        _delegate.evictedObject(store, ident, servant, cookie);
    }

    public void
    invokedObject(Freeze.ObjectStore store, Ice.Identity ident, Ice.Object servant, boolean mutating, boolean idle,
                  Ice.LocalObject cookie)
    {
        _delegate.invokedObject(store, ident, servant, mutating, idle, cookie);
    }

    public void
    destroy()
    {
        _delegate.destroy();
    }

    int
    getLastEvictedValue()
    {
        int result = _lastEvictedValue;
        _lastEvictedValue = -1;
        return result;
    }

    void
    clearLastEvictedValue()
    {
        _lastEvictedValue = -1;
    }

    private Freeze.PersistenceStrategy _delegate;
    private int _lastEvictedValue = -1;
}
