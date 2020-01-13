//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace Ice.hold
{
    public sealed class Hold : Test.IHold
    {
        private static void test(bool b)
        {
            if (!b)
            {
                throw new System.Exception();
            }
        }

        public Hold(Timer timer, ObjectAdapter adapter)
        {
            _timer = timer;
            _adapter = adapter;
        }

        public void
        putOnHold(int milliSeconds, Current current)
        {
            if (milliSeconds < 0)
            {
                _adapter.Hold();
            }
            else if (milliSeconds == 0)
            {
                _adapter.Hold();
                _adapter.Activate();
            }
            else
            {
                _timer.schedule(() =>
                {
                    try
                    {
                        putOnHold(0, null);
                    }
                    catch (ObjectAdapterDeactivatedException)
                    {
                    }
                }, milliSeconds);
            }
        }

        public void
        waitForHold(Current current)
        {
            _timer.schedule(() =>
            {
                try
                {
                    current.Adapter.WaitForHold();
                    current.Adapter.Activate();
                }
                catch (ObjectAdapterDeactivatedException)
                {
                    //
                    // This shouldn't occur. The test ensures all the waitForHold timers are
                    // finished before shutting down the communicator.
                    //
                    test(false);
                }
            }, 0);
        }

        public int
        set(int value, int delay, Current current)
        {
            System.Threading.Thread.Sleep(delay);

            lock (this)
            {
                int tmp = _last;
                _last = value;
                return tmp;
            }
        }

        public void
        setOneway(int value, int expected, Current current)
        {
            lock (this)
            {
                test(_last == expected);
                _last = value;
            }
        }

        public void
        shutdown(Current current)
        {
            _adapter.Hold();
            _adapter.Communicator.shutdown();
        }

        private ObjectAdapter _adapter;
        private int _last = 0;
        private Timer _timer;
    }
}
