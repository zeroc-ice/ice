// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class NonRepeatable extends Exception
{
    public
    NonRepeatable(Ice.RuntimeException ex)
    {
        _ex = ex;
    }

    public Ice.RuntimeException
    get()
    {
        return _ex;
    }

    private Ice.RuntimeException _ex;
}
