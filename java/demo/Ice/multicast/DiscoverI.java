// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class DiscoverI extends _DiscoverDisp
{
    DiscoverI(Ice.ObjectPrx obj)
    {
        _obj = obj;
    }

    public void
    lookup(DiscoverReplyPrx reply, Ice.Current current)
    {
        try
        {
            reply.reply(_obj);
        }
        catch(Ice.LocalException ex)
        {
            // Ignore
        }
    }

    private Ice.ObjectPrx _obj;
}
