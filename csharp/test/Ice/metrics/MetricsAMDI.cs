//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class MetricsAsync : IMetricsAsync
    {
        public ValueTask OpAsync(Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask FailAsync(Current current)
        {
            current.Connection!.Close(ConnectionClose.Forcefully);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask OpWithUserExceptionAsync(Current current) => throw new UserEx("custom UserEx message");

        public ValueTask OpWithRequestFailedExceptionAsync(Current current) =>
            throw new ObjectNotExistException(current);

        public ValueTask OpWithLocalExceptionAsync(Current current) =>
            throw new InvalidConfigurationException("fake");

        public ValueTask OpWithUnknownExceptionAsync(Current current) => throw new ArgumentOutOfRangeException();

        public ValueTask OpByteSAsync(byte[] bs, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask<IObjectPrx?> GetAdminAsync(Current current) =>
            new ValueTask<IObjectPrx?>(current.Adapter.Communicator.GetAdmin());

        public ValueTask ShutdownAsync(Current current)
        {
            current.Adapter.Communicator.ShutdownAsync();
            return new ValueTask(Task.CompletedTask);
        }
    }
}
