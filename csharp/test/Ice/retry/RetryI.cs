//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

namespace ZeroC.Ice.Test.Retry
{
    public sealed class Retry : IRetry
    {
        public void Op(bool kill, Current current)
        {
            if (kill)
            {
                if (current.Connection != null)
                {
                    current.Connection.Close(ConnectionClose.Forcefully);
                }
                else
                {
                    throw new ConnectionLostException();
                }
            }
        }

        public int OpIdempotent(int nRetry, Current current)
        {
            if (nRetry > _counter)
            {
                ++_counter;
                throw new ConnectionLostException();
            }
            int counter = _counter;
            _counter = 0;
            return counter;
        }

        public void OpNotIdempotent(Current current) => throw new ConnectionLostException();

        public void OpSystemException(Current c) => throw new SystemFailure();

        public void Sleep(int delay, Current c) => Thread.Sleep(delay);

        public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();

        private int _counter;
    }
}
