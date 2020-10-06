// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
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
            int[] delays = new int[] { 0, 1, 10000 };
            if (nRetry > _counter)
            {
                throw new SystemFailure(RetryPolicy.AfterDelay(TimeSpan.FromMilliseconds(delays[_counter++ % 3])));
            }
            int counter = _counter;
            _counter = 0;
            return counter;
        }

        public int OpAfterDelay(int retries, int delay, Current current)
        {
            if (retries > _counter)
            {
                _counter++;
                throw new SystemFailure(RetryPolicy.AfterDelay(TimeSpan.FromMilliseconds(delay)));
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

    public sealed class Replicated : IReplicated
    {
        private bool _fail;
        public Replicated(bool fail) => _fail = fail;
        public void OtherReplica(Current current)
        {
            if (_fail)
            {
                throw new SystemFailure(RetryPolicy.OtherReplica);
            }
        }
    }

    public sealed class NonReplicated : INonReplicated
    {
        public void OtherReplica(Current current) => throw new SystemFailure(RetryPolicy.OtherReplica);
    }
}
