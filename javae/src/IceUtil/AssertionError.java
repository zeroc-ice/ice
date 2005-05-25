// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
