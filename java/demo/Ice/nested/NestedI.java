// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
