//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public final class Time {
    public static long currentMonotonicTimeMillis() {
        return System.nanoTime() / 1000000;
    }
}
