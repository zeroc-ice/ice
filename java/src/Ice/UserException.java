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
    __write(IceInternal.BasicStream __os);

    public abstract void
    __read(IceInternal.BasicStream __is);
}
