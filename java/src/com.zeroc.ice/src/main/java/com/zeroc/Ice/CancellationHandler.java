// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

interface CancellationHandler {
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, LocalException ex);
}
