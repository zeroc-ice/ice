//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;

using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice
{
    internal class ConnectionRequestHandler : IRequestHandler
    {
        public ValueTask<Task<IncomingResponseFrame>?> SendRequestAsync(
            OutgoingRequestFrame outgoingRequestFrame,
            bool oneway,
            bool synchronous,
            IInvocationObserver? observer,
            CancellationToken cancel) =>
            _connection.SendRequestAsync(outgoingRequestFrame, oneway, _compress, synchronous, observer, cancel);

        public Connection GetConnection() => _connection;

        public ConnectionRequestHandler(Connection connection, bool compress)
        {
            _connection = connection;
            _compress = compress;
        }

        private readonly Connection _connection;
        private readonly bool _compress;
    }
}
