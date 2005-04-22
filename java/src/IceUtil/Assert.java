// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Assert
{
    public static boolean
    Assert(boolean b)
    {
	//
	// The JVM ignores exceptions raised in finalizers, therefore finalizers
	// that use assertions should call this method. For example:
	//
	// protected void finalize() throws Throwable
	// {
	//     assert(IceUtil.Assert.Assert(condition));
	// }
	//
	// Notice that this method is called from within an assert statement,
	// which avoids the cost of the call if assertions are disabled.
	//
	if(!b)
	{
	    //
	    // Create a Throwable to obtain the stack trace.
	    //
	    Throwable t = new Throwable();
	    StackTraceElement[] trace = t.getStackTrace();
	    if(trace.length > 1)
	    {
		//
		// Skip the first frame, which represents this method.
		//
		System.err.println("Assertion failure:");
		for(int i = 1; i < trace.length; ++i)
		{
		    System.err.println("\tat " + trace[i]);
		}
	    }
	    else
	    {
		System.err.println("Assertion failure (no stack trace information)");
	    }
	}
	return true;
    }
}
