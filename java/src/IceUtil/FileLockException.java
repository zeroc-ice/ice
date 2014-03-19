// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public class FileLockException extends RuntimeException implements Cloneable
{
    public FileLockException()
    {
    }

    public FileLockException(String path)
    {
        this.path = path;
    }

    public FileLockException(Throwable cause)
    {
        super(cause);
    }

    public FileLockException(String path, Throwable cause)
    {
        super(cause);
        this.path = path;
    }

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

    public String
    ice_name()
    {
        return "IceUtil::FileLockException";
    }

    public String
    toString()
    {
        java.io.StringWriter sw = new java.io.StringWriter();
        java.io.PrintWriter pw = new java.io.PrintWriter(sw);
        IceUtilInternal.OutputBase out = new IceUtilInternal.OutputBase(pw);
        out.setUseTab(false);
        out.print(getClass().getName());
        out.inc();
        IceInternal.ValueWriter.write(this, out);
        pw.flush();
        return sw.toString();
    }

    public String path;
}
