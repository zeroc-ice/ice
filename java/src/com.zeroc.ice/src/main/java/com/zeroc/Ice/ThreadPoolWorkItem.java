// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

interface ThreadPoolWorkItem {
    void execute(ThreadPoolCurrent current);
}
