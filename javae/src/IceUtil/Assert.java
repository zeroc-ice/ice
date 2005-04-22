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
	    System.err.println("Assertion failure:");
	    t.printStackTrace(System.err);
	}
	return true;
    }
}
