// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


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
