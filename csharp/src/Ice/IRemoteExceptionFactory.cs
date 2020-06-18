//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>IRemoteExceptionFactory is a publicly visible Ice internal interface used by InputStream and
    /// implemented by the generated code.</summary>
    public interface IRemoteExceptionFactory
    {
        /// <summary>Creates a new instance of the associated remote exception class.</summary>
        /// <param name="message">The message that describes the remote exception.</param>
        /// <returns>The new remote exception.</returns>
        RemoteException Create(string? message);
    }
}
