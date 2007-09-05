// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

final public class Time
{
    static public long
    currentMonotonicTimeMillis()
    {
        //
        // The jdk 1.4 doesn't support also supports a wall
        // clock. That's the best we can use.
        //
        return System.currentTimeMillis();
    }
};
