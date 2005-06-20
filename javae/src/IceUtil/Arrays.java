// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Arrays
{
    public static int
    search(Object[] array, Object item)
    {
	//
	// XXX Array is sorted, implement as binary search.
	//
	for(int i = 0 ; i < array.length ; ++i)
	{
	    if(array[i].equals(item))
	    {
		return i;
	    }
	}
	return -1;
    }

    public static int
    search(java.util.Vector vector, Object item)
    {
	for(int i = 0 ; i < vector.size() ; ++i)
	{
	    if(vector.elementAt(i).equals(item))
	    {
		return i;
	    }
	}
	return -1;
    }

    public static void
    sort(java.util.Vector vector)
    {
	//
	// Bubble sort. This is only used to sequences of endpoints, which for embedded applications will be quite
	// short.
	//
	for(int i = 0; i < vector.size() ; ++i)
	{
	    for(int j = 0; j < vector.size() -1 ; ++j)
	    {
		if(((IceUtil.Comparable)vector.elementAt(j)).compareTo(vector.elementAt(j + 1)) > 0)
		{
		    java.lang.Object t = vector.elementAt(j + 1);
		    vector.setElementAt(vector.elementAt(j), j + 1);
		    vector.setElementAt(t, j);
		}
	    }
	}
    }

    public static boolean
    equals(java.lang.Object[] a1, java.lang.Object[] a2)
    {
	//
	// If they are both null then they are equal.
	//
	if(a1 == null && a2 == null)
	{
	    return true;
	}

	//
	// If one of them is null but the other is not, then they are not equal. This validity of this 'if'
	// statement is order dependent on the previous 'if' statement.
	//
	if(a1 == null || a2 == null)
	{
	    return false;
	}

	if(a1.length != a2.length)
	{
	    return false;
	}

	for(int i = 0; i < a1.length; ++i)
	{
	    if(!a1[i].equals(a2[i]))
	    {
		return false;
	    }
	}
	return true;
    }
}
