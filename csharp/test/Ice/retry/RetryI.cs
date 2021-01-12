// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public sealed class Retry : IRetry
    {
        public void Op(bool kill, Current current, CancellationToken cancel)
        {
            if (kill)
            {
                current.Connection.AbortAsync();
            }
        }

        public int OpIdempotent(int nRetry, Current current, CancellationToken cancel)
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

        public int OpAfterDelay(int retries, int delay, Current current, CancellationToken cancel)
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

        public void OpBidirRetry(IBidirPrx bidir, Current current, CancellationToken cancel)
        {
            bidir = bidir.Clone(fixedConnection: current.Connection);
            try
            {
                bidir.OtherReplica(cancel: CancellationToken.None);
                TestHelper.Assert(false);
            }
            catch (ObjectNotExistException)
            {
            }

            // With Ice1 the exception is not retryable, with Ice2 we can retry using the existing connection
            // because the exception uses the AfterDelay retry policy.
            try
            {
                bidir.AfterDelay(2, cancel: CancellationToken.None);
                TestHelper.Assert(current.Protocol == Protocol.Ice2);
            }
            catch (ObjectNotExistException)
            {
                TestHelper.Assert(current.Protocol == Protocol.Ice1);
            }
        }

        public void OpNotIdempotent(Current current, CancellationToken cancel) =>
            throw new ConnectionLostException(RetryPolicy.AfterDelay(TimeSpan.Zero));

        public void OpSystemException(Current current, CancellationToken cancel) => throw new SystemFailure();

        public void Sleep(int delay, Current current, CancellationToken cancel) => Thread.Sleep(delay);

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public void OpWithData(int retries, int delay, byte[] data, Current current, CancellationToken cancel)
        {
            if (retries > _counter++)
            {
                throw new SystemFailure(RetryPolicy.AfterDelay(TimeSpan.FromMilliseconds(delay)));
            }
            _counter = 0;
        }

        private int _counter;
    }

    public sealed class Replicated : IReplicated
    {
        private bool _fail;
        public Replicated(bool fail) => _fail = fail;
        public void OtherReplica(Current current, CancellationToken cancel)
        {
            if (_fail)
            {
                throw new SystemFailure(RetryPolicy.OtherReplica);
            }
        }
    }

    public sealed class NonReplicated : INonReplicated
    {
        public void OtherReplica(Current current, CancellationToken cancel) =>
            throw new SystemFailure(RetryPolicy.OtherReplica);
    }
}
