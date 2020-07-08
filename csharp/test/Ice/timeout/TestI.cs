//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Timeout
{
    internal class Timeout : ITimeout
    {
        public void op(Current current)
        {
        }

        public void sendData(byte[] seq, Current current)
        {
        }

        public void sleep(int to, Current current)
        {
            if (current.Connection == null)
            {
                // Ensure the collocated dispatch is canceled when the invocation is canceled because of the invocation
                // timeout.
                try
                {
                    Task.Delay(to, current.CancellationToken).Wait();
                    TestHelper.Assert(false);
                }
                catch (TaskCanceledException)
                {
                }
            }
            else
            {
                Thread.Sleep(to);
            }
        }
    }

    internal class Controller : IController
    {
        public Controller(Communicator communicator)
        {
            _adapter = communicator.CreateObjectAdapter("TestAdapter", serializeDispatch: true);
            _adapter.Add("timeout", new Timeout());
            _adapter.ActivateAsync();
        }

        public void holdAdapter(int to, Current current)
        {
            _adapter.Dispose();
            _adapter = _adapter.Communicator.CreateObjectAdapter("TestAdapter", serializeDispatch: true);
            _adapter.Add("timeout", new Timeout());

            if (to >= 0)
            {
                Task.Delay(1000).ContinueWith(t => _adapter.ActivateAsync());
            }
        }

        public void resumeAdapter(Current current) => _adapter.ActivateAsync();

        public void shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private ObjectAdapter _adapter;
    }
}
