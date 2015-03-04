// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// The System.Threading.Monitor class in the Compact Framework does not
// support the Wait/Pulse/PulseAll methods, so we implement our own
// Monitor here.
//
// For non-CF builds, our Monitor simply wraps the standard Monitor class.
//
// All code that uses a monitor for waiting/signaling must now use the
// following locking style:
//
// IceUtilInternal.Monitor mon = new IceUtilInternal.Monitor();
// mon.Lock();
// try
// {
//     // Wait or Notify ...
// }
// finally
// {
//     mon.Unlock();
// }
//
namespace IceUtilInternal
{
#if COMPACT

    using System;
    using System.Collections.Generic;
    using System.Diagnostics;

    public class Monitor
    {
        public Monitor()
        {
            _eventPool = null;
            _mutex = new System.Threading.Mutex();
            _lockCount = 0;
        }

        public void Lock()
        {
            _mutex.WaitOne();
            _lockCount++; // Keep track of recursive locks.
        }

        public void Unlock()
        {
            _lockCount--; // Keep track of recursive locks.
            _mutex.ReleaseMutex();
        }

        public bool TryLock()
        {
            bool result = _mutex.WaitOne(0, false);
            if(result)
            {
                _lockCount++; // Keep track of recursive locks.
            }
            return result;
        }

        public void Wait()
        {
            if(_waitQueue == null)
            {
                _waitQueue = new Queue<LockerEvent>();
            }

            LockerEvent e = acquireEvent();
            try
            {
                _waitQueue.Enqueue(e);

                //
                // Preserve the lock count until we reaquire the lock.
                //
                int lockCount = _lockCount;
                _lockCount = 0;

                //
                // Fully release the lock.
                //
                for(int i = 0; i < lockCount; ++i)
                {
                    _mutex.ReleaseMutex();
                }

                //
                // Wait for the event to be set.
                //
                e.ev.WaitOne();

                //
                // Reacquire the lock the same number of times.
                //
                for(int i = 0; i < lockCount; ++i)
                {
                    _mutex.WaitOne();
                }

                _lockCount = lockCount;

                Debug.Assert(e.notified);
            }
            finally
            {
                releaseEvent(e);
            }
        }

        public bool TimedWait(int timeout)
        {
            if(_waitQueue == null)
            {
                _waitQueue = new Queue<LockerEvent>();
            }

            LockerEvent e = acquireEvent();
            try
            {
                _waitQueue.Enqueue(e);

                //
                // Preserve the lock count until we reaquire the lock.
                //
                int lockCount = _lockCount;
                _lockCount = 0;

                //
                // Fully release the lock.
                //
                for(int i = 0; i < lockCount; ++i)
                {
                    _mutex.ReleaseMutex();
                }

                //
                // Wait for the event to be set or the timeout to expire.
                //
                e.ev.WaitOne(timeout, false);

                //
                // Reacquire the lock the same number of times.
                //
                for(int i = 0; i < lockCount; ++i)
                {
                    _mutex.WaitOne();
                }

                _lockCount = lockCount;
                return e.notified;
            }
            finally
            {
                releaseEvent(e);
            }
        }

        public void Notify()
        {
            if(_waitQueue != null)
            {
                while(_waitQueue.Count > 0)
                {
                    //
                    // Set the first event in the wait queue.
                    //
                    LockerEvent h = _waitQueue.Dequeue();
                    h.notified = true;
                    h.ev.Set();
                }
            }
        }

        public void NotifyAll()
        {
            if(_waitQueue != null)
            {
                //
                // Set all the events in the wait queue.
                //
                foreach(LockerEvent h in _waitQueue)
                {
                    h.notified = true;
                    h.ev.Set();
                }
                _waitQueue.Clear();
            }
        }

        private LockerEvent acquireEvent()
        {
            if(_eventPool == null)
            {
                return new LockerEvent();
            }
            else
            {
                LockerEvent l = _eventPool;
                _eventPool = _eventPool.next;
                l.Reset();
                l.next = null;
                return l;
            }
        }

        private void releaseEvent(LockerEvent e)
        {
            e.next = _eventPool;
            _eventPool = e;
        }

        internal class LockerEvent
        {
            internal System.Threading.EventWaitHandle ev;
            internal bool notified;
            internal LockerEvent next;

            internal LockerEvent()
            {
                ev = new System.Threading.ManualResetEvent(false);
                next = null;
            }

            internal void Reset()
            {
                ev.Reset();
                notified = false;
            }
        }

        private Queue<LockerEvent> _waitQueue;
        private LockerEvent _eventPool;
        private System.Threading.Mutex _mutex;
        private int _lockCount;
    }

#else

    //
    // This implementation is just a wrapper around System.Threading.Monitor.
    //
    public class Monitor
    {
        public void Lock()
        {
            System.Threading.Monitor.Enter(this);
        }

        public void Unlock()
        {
            System.Threading.Monitor.Exit(this);
        }

        public bool TryLock()
        {
            return System.Threading.Monitor.TryEnter(this);
        }

        public void Wait()
        {
            System.Threading.Monitor.Wait(this);
        }

        public bool TimedWait(int timeout)
        {
            return System.Threading.Monitor.Wait(this, timeout);
        }

        public void Notify()
        {
            System.Threading.Monitor.Pulse(this);
        }

        public void NotifyAll()
        {
            System.Threading.Monitor.PulseAll(this);
        }
    }

#endif
}
