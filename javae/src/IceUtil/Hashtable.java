// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Hashtable
{
    static public boolean
    equals(java.util.Hashtable h1, java.util.Hashtable h2)
    {
	if(h1.size() != h2.size())
	{
	    return false;
	}

	java.util.Enumeration e1 = h1.keys();
	boolean mismatch = false;
	while(e1.hasMoreElements() && !mismatch)
	{
	    java.lang.Object k = e1.nextElement();
	    java.lang.Object v1 = h1.get(k);
	    java.lang.Object v2 = h2.get(k);
	    if(v1 == v2)
	    {
		continue;
	    }
	    else if(v1 != null && v1.equals(v2))
	    {
		continue;
	    }
	    mismatch = true;
	}
	return !mismatch;
    }
}
