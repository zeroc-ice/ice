// Copyright (c) ZeroC, Inc.

namespace Ice.retry
{
        using System.Threading;

        public sealed class RetryI : Test.RetryDisp_
        {
            public RetryI()
            {
            }

            public override void op(bool kill, Ice.Current current)
            {
                if (kill)
                {
                    if (current.con != null)
                    {
                        current.con.abort();
                    }
                    else
                    {
                        throw new Ice.ConnectionLostException();
                    }
                }
            }

            public override int opIdempotent(int nRetry, Ice.Current current)
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

            public override void opNotIdempotent(Ice.Current current)
            {
                throw new Ice.ConnectionLostException();
            }

            public override void sleep(int delay, Ice.Current c)
            {
                Thread.Sleep(delay);
            }

            public override void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            private int _counter;
        }
    }

