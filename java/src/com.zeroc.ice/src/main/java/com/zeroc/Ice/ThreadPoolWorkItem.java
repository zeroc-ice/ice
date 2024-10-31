//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

interface ThreadPoolWorkItem {
    void execute(ThreadPoolCurrent current);
}
