//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Metrics
{
    public sealed class Controller : IController
    {
        private readonly TaskScheduler _scheduler;
        private readonly SemaphoreSlim _semaphore = new SemaphoreSlim(0);

        public Controller(TaskScheduler scheduler) => _scheduler = scheduler;

        public void Hold(Current current) =>
            _ = Task.Factory.StartNew(() => _semaphore.Wait(), default, TaskCreationOptions.None, _scheduler);

        public void Resume(Current current) => _semaphore.Release();

    }
}
