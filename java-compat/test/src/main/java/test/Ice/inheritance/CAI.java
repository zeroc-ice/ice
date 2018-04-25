// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.inheritance;

import test.Ice.inheritance.Test.MA.CA;
import test.Ice.inheritance.Test.MA.CAPrx;

public final class CAI extends CA
{
    public
    CAI()
    {
    }

    @Override
    public CAPrx
    caop(CAPrx p, Ice.Current current)
    {
        return p;
    }
}
