// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
