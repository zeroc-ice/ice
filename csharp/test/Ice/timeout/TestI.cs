// Copyright (c) ZeroC, Inc. All rights reserved.

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
    }

    internal class Controller : IController
    {
        private readonly TaskScheduler _scheduler;
        private readonly SemaphoreSlim _semaphore = new SemaphoreSlim(0);

        public Controller(TaskScheduler scheduler) => _scheduler = scheduler;

        public void HoldAdapter(int to, Current current, CancellationToken cancel)
        {
            Task.Factory.StartNew(() => _semaphore.Wait(), default, TaskCreationOptions.None, _scheduler);
            if (to >= 0)
            {
                Task.Delay(to, cancel).ContinueWith(t => _semaphore.Release(), TaskScheduler.Default);
            }
        }

        public void ResumeAdapter(Current current, CancellationToken cancel) => _ = _semaphore.Release();

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Adapter.Communicator.ShutdownAsync();
    }
}
