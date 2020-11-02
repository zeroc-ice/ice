// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Timeout
{
    internal class Timeout : ITimeout
    {
        public void Op(Current current, CancellationToken cancel)
        {
        }

        public void SendData(byte[] seq, Current current, CancellationToken cancel)
        {
        }

        public void Sleep(int to, Current current, CancellationToken cancel)
        {
            if (current.Connection == null)
            {
                // Ensure the collocated dispatch is canceled when the invocation is canceled because of the invocation
                // timeout.
                try
                {
                    Task.Delay(to, cancel).Wait(cancel);
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

        public bool CheckDeadline(Current current, CancellationToken cancel)
        {
            if (current.Protocol == Protocol.Ice2)
            {
                return current.Deadline ==
                    DateTime.UnixEpoch + TimeSpan.FromMilliseconds(long.Parse(current.Context["deadline"]));
            }
            return true;
        }
    }
}
