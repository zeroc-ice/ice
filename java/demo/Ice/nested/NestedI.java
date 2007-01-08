// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

class NestedI extends _NestedDisp
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
