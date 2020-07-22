//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    public interface IAcceptor : IDisposable
    {
        Endpoint Endpoint { get; }

        /// <summary>Accepts a new transport</summary>
        ValueTask<ITransceiver> AcceptAsync();

        string ToDetailedString();
    }
}
