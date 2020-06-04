//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;

namespace ZeroC.Ice.Test.Retry
{
    public sealed class Retry : IRetry
    {
        public void op(bool kill, Current current)
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

        public int opIdempotent(int nRetry, Current current)
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

        public void opNotIdempotent(Current current) => throw new ConnectionLostException();

        public void opSystemException(Current c) => throw new SystemFailure();

        public void sleep(int delay, Current c) => Thread.Sleep(delay);

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        private int _counter;
    }
}
