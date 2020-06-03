//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMD.Metrics
{
    public sealed class Controller : IController
    {
        public Controller(Func<ObjectAdapter> factory)
        {
            _factory = factory;
            _adapter = factory();
            _adapter.Activate();
        }

        public void hold(Current current)
        {
            _adapter.Destroy();
            _adapter = _factory(); // Recreate the adapter without activating it
        }

        public void resume(Current current) => _adapter.Activate();

        private readonly Func<ObjectAdapter> _factory;
        private ObjectAdapter _adapter;
    };

    public sealed class Metrics : IMetrics
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

        public IObjectPrx? getAdmin(Current current)
        {
            TestHelper.Assert(current != null);
            return current.Adapter.Communicator.GetAdmin();
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
