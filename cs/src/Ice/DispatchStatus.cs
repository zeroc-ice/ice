// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

namespace IceInternal
{

using System.Diagnostics;

public sealed class DispatchStatus
{
    private static DispatchStatus[] __values;
    private int __value;
    
    public const int _DispatchOK = 0;
    public static readonly DispatchStatus DispatchOK
	= new DispatchStatus(_DispatchOK);
    
    public const int _DispatchUserException = 1;
    public static readonly DispatchStatus DispatchUserException
	= new DispatchStatus(_DispatchUserException);
    
    public const int _DispatchObjectNotExist = 2;
    public static readonly DispatchStatus DispatchObjectNotExist
	= new DispatchStatus(_DispatchObjectNotExist);
    
    public const int _DispatchFacetNotExist = 3;
    public static readonly DispatchStatus DispatchFacetNotExist
	= new DispatchStatus(_DispatchFacetNotExist);
    
    public const int _DispatchOperationNotExist = 4;
    public static readonly DispatchStatus DispatchOperationNotExist
	= new DispatchStatus(_DispatchOperationNotExist);
    
    public const int _DispatchUnknownLocalException = 5;
    public static readonly DispatchStatus DispatchUnknownLocalException
	= new DispatchStatus(_DispatchUnknownLocalException);
    
    public const int _DispatchUnknownUserException = 6;
    public static readonly DispatchStatus DispatchUnknownUserException
	= new DispatchStatus(_DispatchUnknownUserException);
    
    public const int _DispatchUnknownException = 7;
    public static readonly DispatchStatus DispatchUnknownException
	= new DispatchStatus(_DispatchUnknownException);
    
    //
    // "Pseudo dispatch status", used internally only to indicate
    // async dispatch.
    //
    public const int _DispatchAsync = 8;
    public static readonly DispatchStatus DispatchAsync
	= new DispatchStatus(_DispatchAsync);
    
    public static DispatchStatus
    convert(int val)
    {
	Debug.Assert(val < 9);
	return __values[val];
    }
    
    public int
    val()
    {
	return __value;
    }
    
    private DispatchStatus(int val)
    {
	__value = val;
	__values[val] = this;
    }
    static DispatchStatus()
    {
	__values = new DispatchStatus[9];
    }
}

}
