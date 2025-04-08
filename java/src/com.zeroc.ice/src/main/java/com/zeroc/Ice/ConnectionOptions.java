// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

// All the timeouts are in seconds. A timeout <= 0 means an infinite timeout.

public record ConnectionOptions(
        int connectTimeout,
        int closeTimeout,
        int idleTimeout,
        boolean enableIdleCheck,
        int inactivityTimeout,
        int maxDispatches) {
}
