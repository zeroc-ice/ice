// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Metrics
{
    internal sealed class Metrics : IAsyncMetrics
    {
        public ValueTask OpAsync(Current current, CancellationToken cancel) => default;

        public ValueTask FailAsync(Current current, CancellationToken cancel)
        {
            _ = current.Connection.AbortAsync();
            return default;
        }

        public ValueTask OpWithUserExceptionAsync(Current current, CancellationToken cancel) =>
            throw new UserEx("custom UserEx message");

        public ValueTask OpWithRequestFailedExceptionAsync(Current current, CancellationToken cancel) =>
            throw new ObjectNotExistException();

        public ValueTask OpWithLocalExceptionAsync(Current current, CancellationToken cancel) =>
            throw new InvalidConfigurationException("fake");

        public ValueTask OpWithUnknownExceptionAsync(Current current, CancellationToken cancel) =>
            throw new ArgumentOutOfRangeException();

        public ValueTask OpByteSAsync(byte[] bs, Current current, CancellationToken cancel) => default;

        public ValueTask<IObjectPrx?> GetAdminAsync(Current current, CancellationToken cancel) =>
            new(current.Communicator.GetAdminAsync(cancel: cancel));

        public ValueTask ShutdownAsync(Current current, CancellationToken cancel)
        {
            _ = current.Communicator.ShutdownAsync();
            return default;
        }
    }
}
