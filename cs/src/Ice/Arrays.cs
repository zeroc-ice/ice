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

    public sealed class Arrays
    {
	public static bool Equals(object[] arr1, object[] arr2)
        {
	    if(object.ReferenceEquals(arr1, arr2))
	    {
		return true;
	    }

	    if(arr1.Length == arr2.Length)
	    {
		for(int i = 0; i < arr1.Length; i++)
		{
		    if(!arr1[i].Equals(arr2[i]))
		    {
			return false;
		    }
		}

		return true;
	    }

	    return false;	    
	}
    }

}
