//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

interface CancellationHandler {
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex);
}
