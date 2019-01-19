//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceInternal;

public interface CancellationHandler
{
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, com.zeroc.Ice.LocalException ex);
}
