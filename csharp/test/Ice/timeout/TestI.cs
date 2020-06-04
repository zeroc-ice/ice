//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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

        public void sleep(int to, Current current) => Thread.Sleep(to);
    }

    internal class Controller : IController
    {
        public Controller(Communicator communicator)
        {
            _adapter = communicator.CreateObjectAdapter("TestAdapter", serializeDispatch: true);
            _adapter.Add("timeout", new Timeout());
            _adapter.Activate();
        }

        public void holdAdapter(int to, Current current)
        {
            _adapter.Destroy();
            _adapter = _adapter.Communicator.CreateObjectAdapter("TestAdapter", serializeDispatch: true);
            _adapter.Add("timeout", new Timeout());

            if (to >= 0)
            {
                Task.Delay(1000).ContinueWith(t => _adapter.Activate());
            }
        }

        public void resumeAdapter(Current current) => _adapter.Activate();

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        private ObjectAdapter _adapter;
    }
}
