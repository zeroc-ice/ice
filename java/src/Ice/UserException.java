// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class UserException extends Exception implements Cloneable
{
    public java.lang.Object clone()
    {
        java.lang.Object o = null;
        try
        {
            o = super.clone();
        }
        catch(CloneNotSupportedException ex)
        {
            assert false; // Impossible
        }
        return o;
    }

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

    public abstract void
    __write(IceInternal.BasicStream __os);

    public abstract void
    __read(IceInternal.BasicStream __is, boolean __rid);

    public void
    __write(Ice.OutputStream __outS)
    {
        assert(false);
    }

    public void
    __read(Ice.InputStream __inS, boolean __rid)
    {
        assert(false);
    }

    public boolean
    __usesClasses()
    {
        return false;
    }
}
