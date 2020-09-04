//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>Internal delegate used by InputStream to create remote exception instances.</summary>
    /// <param name="message">The message that describes the remote exception.</param>
    /// <returns>The new remote exception.</returns>
    internal delegate RemoteException IceRemoteExceptionFactory(string? message);
}
