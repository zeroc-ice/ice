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
 
    public String
    toString()
    {
	java.io.ByteArrayOutputStream os = new java.io.ByteArrayOutputStream();
	java.io.PrintStream ps = new java.io.PrintStream(os);
        IceUtil.OutputBase out = new IceUtil.OutputBase(ps);
        out.setUseTab(false);
        out.print(getClass().getName());
        out.inc();
        return os.toString();
    }

    private Throwable _cause;
}
