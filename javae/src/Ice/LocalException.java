// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
	return ice_name();
    }

    private Throwable _cause;
}
