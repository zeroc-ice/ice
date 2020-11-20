// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class Controller : IController
    {
        private readonly TaskScheduler _scheduler;
        private readonly SemaphoreSlim _semaphore = new(0);

        public Controller(TaskScheduler scheduler) => _scheduler = scheduler;

        public void Hold(Current current, CancellationToken cancel) =>
            _ = Task.Factory.StartNew(() => _semaphore.Wait(), default, TaskCreationOptions.None, _scheduler);

        public void Resume(Current current, CancellationToken cancel) => _semaphore.Release();
    }
}
