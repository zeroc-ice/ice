//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

namespace Ice.timeout
{
    internal class ActivateAdapterThread
    {
        internal ActivateAdapterThread(ObjectAdapter adapter, int timeout)
        {
            _adapter = adapter;
            _timeout = timeout;
        }

        internal void run()
        {
            _adapter.WaitForHold();
            Thread.Sleep(_timeout);
            _adapter.Activate();
        }

        private ObjectAdapter _adapter;
        private int _timeout;
    }

    internal class Timeout : Test.ITimeout
    {
        public void op(Current current)
        {
        }

        public void sendData(byte[] seq, Current current)
        {
        }

        public void sleep(int to, Current current) => Thread.Sleep(to);
    }

    internal class Controller : Test.IController
    {
        public Controller(ObjectAdapter adapter) => _adapter = adapter;

        public void holdAdapter(int to, Current current)
        {
            _adapter.Hold();
            if (to >= 0)
            {
                ActivateAdapterThread act = new ActivateAdapterThread(_adapter, to);
                Thread thread = new Thread(new ThreadStart(act.run));
                thread.Start();
            }
        }

        public void resumeAdapter(Current current) => _adapter.Activate();

        public void shutdown(Current current) => current.Adapter.Communicator.shutdown();

        private ObjectAdapter _adapter;
    }
}
