// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

public final class Arrays
{
    public static int
    search(Object[] array, Object item)
    {
	//
	// TODO: Array is expected to be sorted so this would be better
	// implemented as a binary search.
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
    equals(boolean[] a1, boolean[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(byte[] a1, byte[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(short[] a1, short[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(int[] a1, int[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(long[] a1, long[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(float[] a1, float[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
    }

    public static boolean
    equals(double[] a1, double[] a2)
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
	    if(a1[i] != a2[i])
	    {
		return false;
	    }
	}
	return true;
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
