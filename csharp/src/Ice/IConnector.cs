// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>A connector holds all the information needed to establish a connection to a remote peer.</summary>
    public interface IConnector
    {
        /// <summary>Creates a connection. The connection may not be fully connected until its
        /// <see cref="Connection.InitializeAsync"/> method is called.</summary>
        /// <param name="connectionId">The connection ID for the new connection.</param>
        /// <param name="preferNonSecure">Indicates whether the connector should prefer creating a non-secure
        /// over secure connection.</param>
        /// <return>The connection.</return>
        Connection Connect(string connectionId, bool preferNonSecure);
    }
}
