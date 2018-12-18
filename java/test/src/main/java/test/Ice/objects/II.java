// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
