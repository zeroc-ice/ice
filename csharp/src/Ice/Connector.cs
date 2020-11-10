// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>A connector holds all the information needed to establish a connection to a remote peer. It creates
    /// a transceiver that can be used to exchange data with the remote peer once the connection is established.
    /// </summary>
    public abstract class Connector
    {
        /// <summary>Gets the <see cref="Endpoint"/> of this connector.</summary>
        public abstract Endpoint Endpoint { get; }

        /// <summary>Creates a transceiver. The transceiver may not be fully connected until its
        /// <see cref="ITransceiver.InitializeAsync"/> method is called.</summary>
        /// <return>The transceiver.</return>
        public abstract Connection Connect(string connectionId);
    }
}
