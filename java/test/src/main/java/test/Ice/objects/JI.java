// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.J;

public final class JI extends com.zeroc.Ice.InterfaceByValue
{
    public JI()
    {
        super(J.ice_staticId());
    }
}
