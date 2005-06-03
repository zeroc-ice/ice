// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using IceInternal;

namespace IceUtil
{

    public sealed class Assert
    {
	//
	// Both Mono (as of 18 May 2005) and .NET guarantee that writes to
	// the Console are safe during system shutdown.
	//
	public static void
	FinalizerAssert(bool b)
	{
#if DEBUG
	    if(!b)
            {
		System.Console.Error.WriteLine("Assertion failure:");
		System.Diagnostics.StackTrace st = new System.Diagnostics.StackTrace(true);
		System.Console.Error.WriteLine(st);
            }
#endif
	}
    }

}
