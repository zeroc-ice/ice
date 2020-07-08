//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class Controller : IController
    {
        public Controller(Func<ObjectAdapter> factory)
        {
            _factory = factory;
            _adapter = factory();
            _adapter.ActivateAsync();
        }

        public void hold(Current current)
        {
            _adapter.Dispose();
            _adapter = _factory(); // Recreate the adapter without activating it
        }

        public void resume(Current current) => _adapter.ActivateAsync();

        private readonly Func<ObjectAdapter> _factory;
        private ObjectAdapter _adapter;
    }
}
