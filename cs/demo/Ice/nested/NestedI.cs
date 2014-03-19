// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

class NestedI : NestedDisp_
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
            proxy.nestedCall(level, _self);
        }
    }
    
    private NestedPrx _self;
}
