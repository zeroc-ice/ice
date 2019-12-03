//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

import org.checkerframework.checker.nullness.qual.Nullable;

public interface ACMMonitor
{
    void add(com.zeroc.Ice.ConnectionI con);
    void remove(com.zeroc.Ice.ConnectionI con);
    void reap(com.zeroc.Ice.ConnectionI con);

    ACMMonitor acm(@Nullable Integer timeout, com.zeroc.Ice.@Nullable ACMClose close,
                   com.zeroc.Ice.@Nullable ACMHeartbeat heartbeat);
    com.zeroc.Ice.ACM getACM();
}
