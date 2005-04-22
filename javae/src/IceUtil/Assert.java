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
    //
    // The JVM ignores exceptions raised in finalizers, therefore finalizers
    // that use assertions should call this method.
    //
    public static void
    FinalizerAssert(boolean b)
    {
	if(!b)
	{
	    //
	    // Create a Throwable to obtain the stack trace.
	    //
	    Throwable t = new Throwable();
	    System.err.println("Assertion failure:");
	    t.printStackTrace(System.err);
	}
    }
}
