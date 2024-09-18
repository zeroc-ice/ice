//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

/**
 * @hidden Kept public because it's used by IceLocatorDiscovery and the android tests.
 */
public final class Time {
    public static long currentMonotonicTimeMillis() {
        return System.nanoTime() / 1000000;
    }
}
