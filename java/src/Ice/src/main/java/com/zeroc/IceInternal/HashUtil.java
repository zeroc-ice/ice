// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public final class HashUtil
{
    public static int
    hashAdd(int hashCode, boolean value)
    {
        return ((hashCode << 5) + hashCode) ^ (value ? 0 : 1);
    }

    public static int
    hashAdd(int hashCode, short value)
    {
        return ((hashCode << 5) + hashCode) ^ (int)(2654435761l * value);
    }

    public static int
    hashAdd(int hashCode, byte value)
    {
        return ((hashCode << 5) + hashCode) ^ (int)(2654435761l * value);
    }

    public static int
    hashAdd(int hashCode, int value)
    {
        return ((hashCode << 5) + hashCode) ^ (int)(2654435761l * value);
    }

    public static int
    hashAdd(int hashCode, long value)
    {
        return ((hashCode << 5) + hashCode) ^ (int)(value ^ (value >>> 32));
    }

    public static int
    hashAdd(int hashCode, float value)
    {
        return ((hashCode << 5) + hashCode) ^ Float.floatToIntBits(value);
    }

    public static int
    hashAdd(int hashCode, double value)
    {
        long v = Double.doubleToLongBits(value);
        return ((hashCode << 5) + hashCode) ^ (int)(v ^ (v >>> 32));
    }

    public static int
    hashAdd(int hashCode, java.lang.Object value)
    {
        if(value != null)
        {
            hashCode = ((hashCode << 5) + hashCode) ^ value.hashCode();
        }
        return hashCode;
    }

    public static int
    hashAdd(int hashCode, boolean[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, byte[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, char[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, double[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, float[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, int[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, long[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, Object[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }

    public static int
    hashAdd(int hashCode, short[] arr)
    {
        return ((hashCode << 5) + hashCode) ^ java.util.Arrays.hashCode(arr);
    }
}
