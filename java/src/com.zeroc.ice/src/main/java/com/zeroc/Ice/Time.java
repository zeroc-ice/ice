// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

/**
 * @hidden Public because it's used by IceLocatorDiscovery and the android tests.
 */
public final class Time {
    public static long currentMonotonicTimeMillis() {
        return System.nanoTime() / 1000000;
    }
}
