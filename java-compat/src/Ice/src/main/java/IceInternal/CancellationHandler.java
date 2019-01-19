//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public interface CancellationHandler
{
    void asyncRequestCanceled(OutgoingAsyncBase outAsync, Ice.LocalException ex);
}
