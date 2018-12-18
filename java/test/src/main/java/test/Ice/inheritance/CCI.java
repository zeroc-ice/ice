// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CAPrx;
import test.Ice.inheritance.Test.MA.CCDisp;
import test.Ice.inheritance.Test.MA.CCPrx;
import test.Ice.inheritance.Test.MB.CBPrx;

public final class CCI implements CCDisp
{
    public CCI()
    {
    }

    @Override
    public CAPrx caop(CAPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CCPrx ccop(CCPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }

    @Override
    public CBPrx cbop(CBPrx p, com.zeroc.Ice.Current current)
    {
        return p;
    }
}
