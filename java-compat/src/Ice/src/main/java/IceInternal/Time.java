// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

final public class Time
{
    static public long
    currentMonotonicTimeMillis()
    {
        return System.nanoTime() / 1000000;
    }
}
