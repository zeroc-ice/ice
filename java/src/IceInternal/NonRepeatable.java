// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package IceInternal;

public class NonRepeatable extends Exception // TODO: Base class?
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
