// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice
{
    // Default implementation of the Process Admin facet.
    internal sealed class Process : IAsyncProcess
    {
        private readonly Communicator _communicator;

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            _ = _communicator.ShutdownAsync(); // we can't wait for shutdown to complete
            return default;
        }

        public async ValueTask WriteMessageAsync(string message, int fd, Current current, CancellationToken cancel)
        {
            switch (fd)
            {
                case 1:
                    await Console.Out.WriteLineAsync(message).ConfigureAwait(false);
                    break;
                case 2:
                    await Console.Error.WriteLineAsync(message).ConfigureAwait(false);
                    break;
            }
        }

        internal Process(Communicator communicator) => _communicator = communicator;
    }
}
