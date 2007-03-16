// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Debug
{
    //
    // The ASSERT member determines whether assertions are enabled.
    // Applications should use the following idiom:
    //
    // if(IceUtil.Debug.ASSERT)
    // {
    //     IceUtil.Debug.Assert(expression);
    // }
    //
    // When ASSERT is set to false, a decent Java compiler will detect that
    // the above block can never be executed and eliminate it from the
    // bytecode.
    //
    public static final boolean ASSERT = false;

    public static void
    Assert(boolean b)
    {
	if(!b)
	{
	    throw new AssertionError();
	}
    }

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
	    t.printStackTrace();
	}
    }
}
