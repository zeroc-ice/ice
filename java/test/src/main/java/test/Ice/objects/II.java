// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.I;

public final class II extends com.zeroc.Ice.InterfaceByValue
{
    public II()
    {
        super(I.ice_staticId());
    }
}
