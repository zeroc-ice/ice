// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
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

    public String
    toString()
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtil.OutputBase out = new IceUtil.OutputBase(pw);
        out.setUseTab(false);
        out.print(getClass().getName());
        out.inc();
        IceInternal.ValueWriter.write(this, out);
        pw.flush();
        return sw.toString();
    }

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
