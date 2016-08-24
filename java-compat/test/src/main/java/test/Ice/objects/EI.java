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


public final class EI extends E
{
    public
    EI()
    {
        super(1, "hello");
    }

    @Override
    public boolean
    checkValues(Ice.Current current)
    {
        return i == 1 && s.equals("hello");
    }
}
