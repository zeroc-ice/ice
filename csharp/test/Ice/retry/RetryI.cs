//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice
{
    namespace retry
    {
        using System.Threading;

        public sealed class RetryI : Test.Retry
        {
            public RetryI()
            {
            }

            public void op(bool kill, Ice.Current current)
            {
                if (kill)
                {
                    if (current.con != null)
                    {
                        current.con.close(Ice.ConnectionClose.Forcefully);
                    }
                    else
                    {
                        throw new Ice.ConnectionLostException();
                    }
                }
            }

            public int opIdempotent(int nRetry, Ice.Current current)
            {
                if (nRetry > _counter)
                {
                    ++_counter;
                    throw new Ice.ConnectionLostException();
                }
                int counter = _counter;
                _counter = 0;
                return counter;
            }

            public void opNotIdempotent(Ice.Current current)
            {
                throw new Ice.ConnectionLostException();
            }

            public void opSystemException(Ice.Current c)
            {
                throw new SystemFailure();
            }

            public void sleep(int delay, Ice.Current c)
            {
                Thread.Sleep(delay);
            }

            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private int _counter;
        }
    }
}
