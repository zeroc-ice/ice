//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class MetricsAsync : IMetricsAsync
    {
        public ValueTask opAsync(Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask failAsync(Current current)
        {
            current.Connection!.Close(ConnectionClose.Forcefully);
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask opWithUserExceptionAsync(Current current) => throw new UserEx();

        public ValueTask opWithRequestFailedExceptionAsync(Current current) =>
            throw new ObjectNotExistException(current);

        public ValueTask opWithLocalExceptionAsync(Current current) =>
            throw new InvalidConfigurationException("fake");

        public ValueTask
        opWithUnknownExceptionAsync(Current current) => throw new ArgumentOutOfRangeException();

        public ValueTask opByteSAsync(byte[] bs, Current current) => new ValueTask(Task.CompletedTask);

        public ValueTask<IObjectPrx?> getAdminAsync(Current current) =>
            new ValueTask<IObjectPrx?>(current.Adapter.Communicator.GetAdmin());

        public ValueTask shutdownAsync(Current current)
        {
            current.Adapter.Communicator.Shutdown();
            return new ValueTask(Task.CompletedTask);
        }
    }
}
