// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

class AsyncStatus {
    public static final int Queued = 0;
    public static final int Sent = 1;
    public static final int InvokeSentCallback = 2;

    private AsyncStatus() {
    }
}
