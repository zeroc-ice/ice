// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class EI extends _EDisp
{
    public
    EI()
    {
    }

    public String
    callE(Ice.Current current)
    {
        return "E";
    }
}
