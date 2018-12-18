// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.objects;

import test.Ice.objects.Test.C;

public final class CI extends C
{
    @Override
    public void ice_preMarshal()
    {
        preMarshalInvoked = true;
    }

    @Override
    public void ice_postUnmarshal()
    {
        postUnmarshalInvoked = true;
    }
}
