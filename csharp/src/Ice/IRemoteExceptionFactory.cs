//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    /// <summary>IRemoteExceptionFactory is a publicly visible Ice internal interface used by InputStream and
    /// implemented by the generated code.</summary>
    public interface IRemoteExceptionFactory
    {
        /// <summary>Creates a new instance of the associated remote exception and reads its fields from the stream.
        /// </summary>
        /// <param name="istr">The input stream.</param>
        /// <returns>The new remote exception.</returns>
        RemoteException Read(InputStream istr);
    }
}
