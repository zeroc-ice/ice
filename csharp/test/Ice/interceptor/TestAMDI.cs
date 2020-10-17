// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Interceptor
{
    public sealed class AsyncMyObject : IAsyncMyObject
    {
        public ValueTask<int> AddAsync(int x, int y, Current current, CancellationToken cancel) =>
            new ValueTask<int>(x + y);
        public ValueTask<int> AddWithRetryAsync(int x, int y, Current current, CancellationToken cancel)
        {
            if (current.Context.TryGetValue("retry", out string? value) && value == "no")
            {
                return new ValueTask<int>(x + y);
            }
            throw new RetryException();
        }
        public ValueTask<int> BadAddAsync(int x, int y, Current current, CancellationToken cancel) =>
            throw new InvalidInputException("badAdd");
        public ValueTask<int> NotExistAddAsync(int x, int y, Current current, CancellationToken cancel) =>
            throw new ObjectNotExistException();
        public ValueTask Op1Async(Current current, CancellationToken cancel) => new ValueTask();
        public ValueTask OpWithBinaryContextAsync(Token token, Current current, CancellationToken cancel) => default;
        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            current.Communicator.ShutdownAsync();
            return default;
        }
    }
}
