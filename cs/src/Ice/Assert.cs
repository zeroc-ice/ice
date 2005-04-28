// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceUtil
{

    public sealed class Assert
    {
	//
	// In C#, it's not safe to call *anything* from within a finalizer
	// because the finalizer may run as part of process shutdown, and
	// order of finalization is undefined. So, we don't run assertions
        // once shutdown has started.
	//
	public static void
	FinalizerAssert(bool b)
	{
	    if(!b && !System.Environment.HasShutdownStarted)
            {
                    System.Console.Error.WriteLine("Assertion failure:");

                    System.Diagnostics.StackTrace st = new System.Diagnostics.StackTrace(true);
	            System.Console.Error.WriteLine(st);
            }
	}
    }

}
