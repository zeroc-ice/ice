// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

import Test.*;

public final class CI extends _CDisp
{
    public
    CI()
    {
    }

    public String
    callA(Ice.Current current)
    {
        return "A";
    }

    public String
    callC(Ice.Current current)
    {
        return "C";
    }
}
