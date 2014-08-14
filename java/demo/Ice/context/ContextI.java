// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ContextI extends _ContextDisp
{
    @Override
    public void
    call(Ice.Current current)
    {
        System.out.print("Type = ");
        String type = current.ctx.get("type");
        if(type != null)
        {
            System.out.print(type);
        }
        else
        {
            System.out.print("None");
        }
        System.out.println();
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        System.out.println("Shutting down...");
        current.adapter.getCommunicator().shutdown();
    }
}
