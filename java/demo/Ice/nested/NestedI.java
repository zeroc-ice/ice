// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

class NestedI extends Nested
{
    NestedI(NestedPrx self)
    {
        _self = self;
    }

    public void
    nested(int level, NestedPrx proxy, Ice.Current current)
    {
        System.out.println("" + level);
        if(--level > 0)
        {
            proxy.nested(level, _self, current.context);
        }
    }

    private NestedPrx _self;
}
