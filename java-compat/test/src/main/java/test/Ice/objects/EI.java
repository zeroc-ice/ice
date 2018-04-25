// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.E;

public final class EI extends E
{
    public EI()
    {
        super(1, "hello");
    }

    public boolean checkValues()
    {
        return i == 1 && s.equals("hello");
    }
}
