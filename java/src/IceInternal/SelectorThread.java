// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class SelectorThread
{
    static public abstract class SocketReadyCallback extends SelectorHandler implements TimerTask
    {
        abstract public SocketStatus socketReady();
        abstract public void socketFinished();

        //
        // The selector thread doesn't unregister the callback when sockectTimeout is called; socketTimeout
        // must unregister the callback either explicitly with unregister() or by shutting down the socket 
        // (if necessary).
        //
        //abstract void socketTimeout();

        protected int _timeout;
        protected SocketStatus _status;
        protected SocketStatus _previousStatus;
    }

    SelectorThread(Instance instance)
    {
        _instance = instance;
        _destroyed = false;
        _selector = new Selector(instance, 0);

        _thread = new HelperThread();
        _thread.start();

        _timer = _instance.timer();
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        IceUtilInternal.Assert.FinalizerAssert(_destroyed);
    }

    public synchronized void
    destroy()
    {
        assert(!_destroyed);
        _destroyed = true;
        _selector.setInterrupt();
    }

    public synchronized void
    _register(SocketReadyCallback cb, SocketStatus status, int timeout)
    {
        assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
        assert(status != SocketStatus.Finished);

        cb._timeout = timeout;
        cb._status = status;
        if(cb._timeout >= 0)
        {
            _timer.schedule(cb, cb._timeout);
        }

        _selector.add(cb, status);
    }

    public synchronized void
    unregister(SocketReadyCallback cb)
    {
        // Note: unregister should only be called from the socketReady() call-back.
        assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
        assert(cb._status != SocketStatus.Finished);

        _selector.remove(cb);
        cb._status = SocketStatus.Finished;
    }

    public synchronized void
    finish(SocketReadyCallback cb)
    {
        assert(!_destroyed); // The selector thread is destroyed after the incoming/outgoing connection factories.
        assert(cb._status != SocketStatus.Finished);

        _selector.remove(cb);
        cb._status = SocketStatus.Finished;

        _finished.add(cb);
        _selector.setInterrupt();
    }

    public void
    joinWithThread()
    {
        if(_thread != null)
        {
            try
            {
                _thread.join();
            }
            catch(InterruptedException ex)
            {
            }
        }
    }

    public void
    run()
    {
        while(true)
        {
            try
            {
                _selector.select();
            }
            catch(java.io.IOException ex)
            {
                Ice.SocketException se = new Ice.SocketException();
                se.initCause(ex);
                //throw se;
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                se.printStackTrace(pw);
                pw.flush();
                String s = "exception in selector thread:\n" + sw.toString();
                _instance.initializationData().logger.error(s);
                continue;
            }
            
            java.util.LinkedList<SocketReadyCallback> readyList = new java.util.LinkedList<SocketReadyCallback>();
            boolean finished = false;

            synchronized(this)
            {
                _selector.checkTimeout();

                if(_selector.isInterrupted())
                {
                    if(_selector.processInterrupt())
                    {
                        continue;
                    }

                    //
                    // There are two possiblities for an interrupt:
                    //
                    // 1. The selector thread has been destroyed.
                    // 2. A callback is being finished
                    //

                    //
                    // Thread destroyed?
                    //
                    if(_destroyed)
                    {
                        break;
                    }

                    do
                    {
                        SocketReadyCallback cb = _finished.removeFirst();
                        cb._previousStatus = SocketStatus.Finished;
                        readyList.add(cb);
                    }
                    while(_selector.clearInterrupt()); // As long as there are interrupts
                    finished = true;
                }
                else
                {
                    SocketReadyCallback cb;
                    while((cb = (SocketReadyCallback)_selector.getNextSelected()) != null)
                    {
                        cb._previousStatus = cb._status;
                        readyList.add(cb);
                    }
                }
            }

            java.util.Iterator<SocketReadyCallback> iter = readyList.iterator();
            while(iter.hasNext())
            {
                SocketStatus status = SocketStatus.Finished;
                SocketReadyCallback cb = iter.next();
                try
                {
                    if(cb._timeout >= 0)
                    {
                        _timer.cancel(cb);
                    }
                    
                    if(finished)
                    {
                        cb.socketFinished();
                    }
                    else
                    {
                        status = cb.socketReady();
                    }
                }
                catch(Ice.LocalException ex)
                {
                    java.io.StringWriter sw = new java.io.StringWriter();
                    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    String s = "exception in selector thread " + _thread.getName() + 
                        " while calling socketReady():\n" + sw.toString();
                    _instance.initializationData().logger.error(s);
                    status = SocketStatus.Finished;
                }

                if(status != SocketStatus.Finished)
                {
                    if(cb.hasMoreData())
                    {
                        _selector.hasMoreData(cb);
                    }

                    if(status != cb._previousStatus)
                    {
                        synchronized(this)
                        {
                            // The callback might have been finished concurrently.
                            if(cb._status != SocketStatus.Finished) 
                            {
                                _selector.update(cb, status);
                                cb._status = status;
                            }
                        }
                    }

                    if(cb._timeout >= 0)
                    {
                        _timer.schedule(cb, cb._timeout);
                    }
                }
            }
        }

        assert(_destroyed);

        _selector.destroy();
    }

    private Instance _instance;
    private boolean _destroyed;
    private Selector _selector;
    private java.util.LinkedList<SocketReadyCallback> _finished = new java.util.LinkedList<SocketReadyCallback>();

    private final class HelperThread extends Thread
    {
        HelperThread()
        {
            String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
            if(threadName.length() > 0)
            {
                threadName += "-";
            }
            setName(threadName + "Ice.SelectorThread");
        }

        public void
        run()
        {
            try
            {
                SelectorThread.this.run();
            }
            catch(Ice.LocalException ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "exception in selector thread " + getName() + ":\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
            catch(java.lang.Exception ex)
            {
                java.io.StringWriter sw = new java.io.StringWriter();
                java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                ex.printStackTrace(pw);
                pw.flush();
                String s = "unknown exception in selector thread " + getName() + ":\n" + sw.toString();
                _instance.initializationData().logger.error(s);
            }
        }
    }

    private HelperThread _thread;
    private Timer _timer;
}
