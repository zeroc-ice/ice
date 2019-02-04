//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface ThreadPoolWorkItem
{
    void execute(ThreadPoolCurrent current);
}
