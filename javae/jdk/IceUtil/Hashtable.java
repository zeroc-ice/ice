// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Hashtable
{
    static public boolean
    equals(java.util.Hashtable h1, java.util.Hashtable h2)
    {
	if(h1 == null)
	{
	    return (h2 == null);
	}
	return h1.equals(h2);
    }
}
