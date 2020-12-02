// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Threading
{
    public sealed class TestIntf : ITestIntf
    {
        private readonly TaskScheduler _scheduler;
        private int _level;
        private readonly object _mutex = new();

        public TestIntf(TaskScheduler scheduler) => _scheduler = scheduler;

        public void PingSync(Current current, CancellationToken cancel)
        {
            if (TaskScheduler.Current != _scheduler &&
                (!current.Context.TryGetValue("scheduler", out string? id) ||
                 TaskScheduler.Current.Id.ToString() != id!))
            {
                throw new TestFailedException(
                    $"unexpected task scheduler from pingSync dispatch: {TaskScheduler.Current}");
            }
            // We add a small delay to ensure collocated calls don't complete synchronously, this is necessary to
            // test that invocation awaited with ConfigureAwait(false) continue using the default scheduler.
            Thread.Sleep(1);
        }

        public async ValueTask PingAsync(Current current, CancellationToken cancel)
        {
            if (TaskScheduler.Current != _scheduler &&
                (!current.Context.TryGetValue("scheduler", out string? id) ||
                 TaskScheduler.Current.Id.ToString() != id!))
            {
                throw new TestFailedException(
                    $"unexpected task scheduler from pingAsync dispatch: {TaskScheduler.Current}");
            }
            await Task.Delay(1, cancel).ConfigureAwait(false);
        }

        public void Concurrent(int level, Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (_level == -1)
                {
                    // Test is done, just return.
                    return;
                }

                ++_level;
                if (_level < level)
                {
                    Monitor.Wait(_mutex);
                    return;
                }
                else if (_level > level)
                {
                    int currentLevel = _level;
                    Monitor.Wait(_mutex);
                    throw new TestFailedException($"task scheduler concurrency level exceeded {currentLevel} > {level}");
                }
            }

            // Wait to ensure no other concurrent calls are dispatched while we reached the given
            // concurrency level.
            Thread.Sleep(200);

            lock (_mutex)
            {
                Monitor.PulseAll(_mutex); // Free waiting threads
                _level = -1;
            }
        }

        public void Reset(Current current, CancellationToken cancel)
        {
            lock (_mutex)
            {
                _level = 0;
            }
        }

        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();
    }
}
