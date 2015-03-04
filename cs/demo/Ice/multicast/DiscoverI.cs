// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Demo;

public class DiscoverI : DiscoverDisp_
{
    public
    DiscoverI(Ice.ObjectPrx obj)
    {
        _obj = obj;
    }

    public override void
    lookup(DiscoverReplyPrx reply, Ice.Current current)
    {
        try
        {
            reply.reply(_obj);
        }
        catch(Ice.LocalException)
        {
            // Ignore
        }
    }

    private Ice.ObjectPrx _obj;
}
