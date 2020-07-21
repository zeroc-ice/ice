//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class Metrics : IMetrics
    {
        public void Op(Current current)
        {
        }

        public void Fail(Current current) => current.Connection!.Close(ConnectionClose.Forcefully);

        public void OpWithUserException(Current current) => throw new UserEx("custom UserEx message");

        public void OpWithRequestFailedException(Current current) => throw new ObjectNotExistException(current);

        public void OpWithLocalException(Current current) => throw new InvalidConfigurationException("fake");

        public void OpWithUnknownException(Current current) => throw new ArgumentOutOfRangeException();

        public void OpByteS(byte[] bs, Current current)
        {
        }

        public IObjectPrx? GetAdmin(Current current) => current.Adapter.Communicator.GetAdmin();

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
    }
}
