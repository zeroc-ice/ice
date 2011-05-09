// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
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
            _waitQueue = new LinkedList<System.Threading.EventWaitHandle>();
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
            //
            // Push an event onto the wait queue. Eventually, a call to Notify or NotifyAll
            // will remove the event from the wait queue and signal it.
            //
            System.Threading.EventWaitHandle e = new System.Threading.AutoResetEvent(false);
            _waitQueue.AddLast(e);

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
            e.WaitOne();

            //
            // Reacquire the lock the same number of times.
            //
            for(int i = 0; i < lockCount; ++i)
            {
                _mutex.WaitOne();
            }

            _lockCount = lockCount;

            //
            // It is safe to close the event now because no other thread will use it (Notify
            // or NotifyAll has already removed the event from the wait queue).
            //
            e.Close();
        }

        public bool TimedWait(int timeout)
        {
            //
            // Push an event onto the wait queue. The event is removed from the queue if
            // Notify or NotifyAll is called, otherwise we have to remove it explicitly.
            // We use a LinkedListNode here because we can remove it in O(1) time.
            //
            System.Threading.EventWaitHandle e = new System.Threading.AutoResetEvent(false);
            LinkedListNode<System.Threading.EventWaitHandle> node =
                new LinkedListNode<System.Threading.EventWaitHandle>(e);
            _waitQueue.AddLast(node);

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
            bool b = e.WaitOne(timeout, false);

            //
            // NOTE: There's a race here if the timeout expired: another thread could
            // acquire the lock and call Notify. In turn, Notify could remove this event
            // from the wait queue and set it. Now we have a situation where the timeout
            // technically expired but the event was actually set. If we still treat this
            // as an expired timeout then the Notify will have been lost.
            //
            // The timeout isn't precise because we also have to wait an indeterminate
            // time to reacquire the lock. The simplest solution therefore is to check
            // the event one more time after acquiring the lock - if it's set now, we
            // act as if the wait succeeded. This might be an issue for a general-purpose
            // monitor implementation, but for Ice it shouldn't cause any problems.
            //

            //
            // Reacquire the lock the same number of times.
            //
            for(int i = 0; i < lockCount; ++i)
            {
                _mutex.WaitOne();
            }

            _lockCount = lockCount;

            //
            // In the case of a timeout, check the event one more time to work around the
            // race condition described above.
            //
            if(!b)
            {
                b = e.WaitOne(0, false);
            }

            //
            // If our event was not signaled, we need to remove it from the wait queue.
            //
            if(!b)
            {
                Debug.Assert(node.List != null); // The node must still be in the wait queue.
                _waitQueue.Remove(node);
            }

            //
            // It is safe to close the event now because no other thread will use it.
            //
            e.Close();

            return b;
        }

        public void Notify()
        {
            if(_waitQueue.Count > 0)
            {
                //
                // Set the first event in the wait queue.
                //
                System.Threading.EventWaitHandle h = _waitQueue.First.Value;
                _waitQueue.RemoveFirst();
                h.Set();
            }
        }

        public void NotifyAll()
        {
            //
            // Set all the events in the wait queue.
            //
            foreach(System.Threading.EventWaitHandle h in _waitQueue)
            {
                h.Set();
            }
            _waitQueue.Clear();
        }

        private LinkedList<System.Threading.EventWaitHandle> _waitQueue;
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
