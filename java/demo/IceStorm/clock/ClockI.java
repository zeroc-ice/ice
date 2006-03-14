// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Demo.*;

public class ClockI extends _ClockDisp
{
    public void
    tick(Ice.Current current)
    {
        System.out.println("tick");
    }
}
