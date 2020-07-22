//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    // TODO: eliminate this class and change Connection to implement IRequestHandler? Or could it be useful for
    // upcoming client interceptors?
    internal class ConnectionRequestHandler : IRequestHandler
    {
        public ValueTask<IncomingResponseFrame> SendRequestAsync(
            OutgoingRequestFrame outgoingRequestFrame,
            bool oneway,
            bool synchronous,
            IInvocationObserver? observer,
            IProgress<bool> progress,
            CancellationToken cancel) =>
            _connection.SendRequestAsync(outgoingRequestFrame,
                                         oneway,
                                         synchronous,
                                         observer,
                                         progress,
                                         cancel);

        public Connection GetConnection() => _connection;

        public ConnectionRequestHandler(Connection connection)
        {
            _connection = connection;
        }

        private readonly Connection _connection;
    }
}
