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


package IceInternal;

public class NonRepeatable extends Exception
{
    public
    NonRepeatable(Ice.LocalException ex)
    {
        _ex = ex;
    }

    public Ice.LocalException
    get()
    {
        return _ex;
    }

    private Ice.LocalException _ex;
}
