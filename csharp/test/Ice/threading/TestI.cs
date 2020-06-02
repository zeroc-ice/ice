//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Threading
{
    public sealed class TestIntf : ITestIntf
    {
        private TaskScheduler _scheduler;
        private int _level;
        private object _mutex = new object();

        public TestIntf(TaskScheduler scheduler)
        {
            _scheduler = scheduler;
        }

        public void pingSync(Current current)
        {
            if (TaskScheduler.Current != _scheduler &&
                (!current.Context.TryGetValue("scheduler", out string? id) ||
                 TaskScheduler.Current.Id.ToString() != id!))
            {
                throw new TestFailedException(
                    $"unexpected task scheduler from pingSync dispatch: {TaskScheduler.Current}");
            }
        }

        public async ValueTask pingAsync(Current current)
        {
            if (TaskScheduler.Current != _scheduler &&
                (!current.Context.TryGetValue("scheduler", out string? id) ||
                 TaskScheduler.Current.Id.ToString() != id!))
            {
                throw new TestFailedException(
                    $"unexpected task scheduler from pingAsync dispatch: {TaskScheduler.Current}");
            }
            await Task.Delay(1).ConfigureAwait(false);
        }

        public void concurrent(int level, Current current)
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
                    Monitor.Wait(_mutex);
                    throw new TestFailedException($"task scheduler concurrency level exceeded {_level} > {level}");
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

        public void reset(Current current)
        {
            lock (_mutex)
            {
                _level = 0;
            }
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();
    }
}
