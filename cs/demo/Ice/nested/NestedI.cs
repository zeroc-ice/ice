// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


class NestedI : Nested
{
    internal NestedI(NestedPrx self)
    {
        _self = self;
    }
    
    public override void nestedCall(int level, NestedPrx proxy, Ice.Current current)
    {
        System.Console.Out.WriteLine("" + level);
        if(--level > 0)
        {
            proxy.nestedCall(level, _self, current.ctx);
        }
    }
    
    private NestedPrx _self;
}
