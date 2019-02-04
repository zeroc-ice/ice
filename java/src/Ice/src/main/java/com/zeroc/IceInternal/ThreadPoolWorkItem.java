//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface ThreadPoolWorkItem
{
    void execute(ThreadPoolCurrent current);
}
