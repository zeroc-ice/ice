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
