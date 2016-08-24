// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA._CADisp;
import test.Ice.inheritance.Test.MA.CAPrx;

public final class CAI implements _CADisp
{
    public CAI()
    {
    }

    @Override
    public CAPrx caop(CAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }
}
