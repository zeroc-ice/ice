// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
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

    public abstract void
    __marshal(Stream __os);

    public abstract void
    __unmarshal(Stream __is);

    public final void
    ice_marshal(String __name, Stream __os)
    {
        __os.startWriteException(__name);
        __marshal(__os);
        __os.endWriteException();
    }
}
