// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public class AssertionError extends Error
{
    public
    AssertionError()
    {
    }

    public
    AssertionError(String message)
    {
	super(message);
    }
}
