// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Ice;

public abstract class LocalException extends RuntimeException
{
    public abstract String
    ice_name();

    public Throwable
    getCause()
    {
	return _cause;
    }

    public Throwable
    initCause(Throwable cause)
    {
	if(_cause != null)
	{
	    throw new IllegalStateException();
	}

	_cause = cause;

	return this;
    }

    public void
    printStackTrace()
    {
	super.printStackTrace();
	if(_cause != null)
	{
	    System.err.println("\nCaused by:");
	    _cause.printStackTrace();
	}
    }

    public void
    printStackTrace(java.io.PrintStream ps)
    {
	super.printStackTrace(ps);
	if(_cause != null)
	{
	    ps.println("\nCaused by:");
	    _cause.printStackTrace(ps);
	}
    }

    public void
    printStackTrace(java.io.PrintWriter pw)
    {
	super.printStackTrace(pw);
	if(_cause != null)
	{
	    pw.println("\nCaused by:");
	    _cause.printStackTrace(pw);
	}
    }

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

    private Throwable _cause;
}
