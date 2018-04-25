// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MB.CBDisp;
import test.Ice.inheritance.Test.MB.CBPrx;

public final class CBI implements CBDisp
{
    public CBI()
    {
    }

    @Override
    public CAPrx caop(CAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CBPrx cbop(CBPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }
}
