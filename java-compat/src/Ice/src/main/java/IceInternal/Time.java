// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        return System.nanoTime() / 1000000;
    }
}
