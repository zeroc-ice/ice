// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.E;
import test.Ice.objects.Test.F;


public final class FI extends F
{
    public
    FI()
    {
    }

    public
    FI(E e)
    {
        super(e, e);
    }

    @Override
    public boolean
    checkValues(Ice.Current current)
    {
        return e1 != null && e1 == e2;
    }
}
