// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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
    nestedCall(int level, NestedPrx proxy, Ice.Current current)
    {
        System.out.println("" + level);
        if(--level > 0)
        {
            proxy.nestedCall(level, _self, current.ctx);
        }
    }

    private NestedPrx _self;
}
