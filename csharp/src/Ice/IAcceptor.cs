//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    /// <summary>An acceptor is used to listen and accept incoming connection requests from clients. It enables
    /// creating a transceiver that can be used to exchange data with the remote peer once the connection is
    /// accepted.</summary>
    public interface IAcceptor : IDisposable
    {
        /// <summary>The listening endpoint. The acceptor endpoint might be different from the endpoint used
        /// to create the acceptor if for example the binding of server socket assigned a port.</summary>
        /// <return>The bound endpoint.</return>
        Endpoint Endpoint { get; }

        /// <summary>Accepts a new connection.</summary>
        /// <return>The accepted transceiver.</return>
        ValueTask<ITransceiver> AcceptAsync();

        /// <summary>Get a detailed description of the acceptor.</summary>
        /// <return>The detailed description.</return>
        string ToDetailedString();
    }
}
