// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Ice;

public abstract class UserException extends Exception
{
    public abstract String
    ice_name();

    public abstract String[]
    __getExceptionIds();

    public abstract void
    __write(Stream out);

    public abstract void
    __read(Stream out);
}
