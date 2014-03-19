// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

//
// NOTE: We don't use the java.util.Timer class for few reasons. The
// Java TimerTask is a class not an interface making it more difficult
// to use. The API is also a bit different, cancel() is a TimerTask
// method not a Timer method and calling purge() on the timer on a
// regular basis is required to allow canceled timer task objects to
// be garbage collected.
//
public final class Timer extends Thread
{
    //
    // Renamed from destroy to _destroy to avoid a deprecation warning caused
    // by the destroy method inherited from Thread.
    //
    public void
    _destroy()
    {
        synchronized(this)
        {
            if(_instance == null)
            {
                return;
            }

            _instance = null;
            notify();

            _tokens.clear();
            _tasks.clear();
        }

        while(true)
        {
            try
            {
                join();
                break;
            }
            catch(java.lang.InterruptedException ex)
            {
            }
        }
    }

    synchronized public void
    schedule(TimerTask task, long delay)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        final Token token = new Token(IceInternal.Time.currentMonotonicTimeMillis() + delay, ++_tokenId, 0, task);

        Token previous = _tasks.put(task, token);
        assert previous == null;
        _tokens.add(token);

        if(token.scheduledTime < _wakeUpTime)
        {
            notify();
        }
    }

    synchronized public void
    scheduleRepeated(TimerTask task, long period)
    {
        if(_instance == null)
        {
            throw new Ice.CommunicatorDestroyedException();
        }

        final Token token = new Token(IceInternal.Time.currentMonotonicTimeMillis() + period, ++_tokenId, period, task);

        Token previous = _tasks.put(task, token);
        assert previous == null;
        _tokens.add(token);

        if(token.scheduledTime < _wakeUpTime)
        {
            notify();
        }
    }

    synchronized public boolean
    cancel(TimerTask task)
    {
        if(_instance == null)
        {
            return false;
        }

        Token token = _tasks.remove(task);
        if(token == null)
        {
            return false;
        }

        _tokens.remove(token);
        return true;
    }

    //
    // Only for use by Instance.
    //
    Timer(IceInternal.Instance instance)
    {
        _instance = instance;

        String threadName = _instance.initializationData().properties.getProperty("Ice.ProgramName");
        if(threadName.length() > 0)
        {
            threadName += "-";
        }
        setName(threadName + "Ice.Timer");

        start();
    }

    protected synchronized void
    finalize()
        throws Throwable
    {
        try
        {
            IceUtilInternal.Assert.FinalizerAssert(_instance == null);
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    public void
    run()
    {
        Token token = null;
        while(true)
        {
            synchronized(this)
            {
                if(_instance != null)
                {
                    //
                    // If the task we just ran is a repeated task, schedule it
                    // again for executation if it wasn't canceled.
                    //
                    if(token != null && token.delay > 0)
                    {
                        if(_tasks.containsKey(token.task))
                        {
                            token.scheduledTime = IceInternal.Time.currentMonotonicTimeMillis() + token.delay;
                            _tokens.add(token);
                        }
                    }
                }
                token = null;

                if(_instance == null)
                {
                    break;
                }

                if(_tokens.isEmpty())
                {
                    _wakeUpTime = Long.MAX_VALUE;
                    while(true)
                    {
                        try
                        {
                            wait();
                            break;
                        }
                        catch(java.lang.InterruptedException ex)
                        {
                        }
                    }
                }

                if(_instance == null)
                {
                    break;
                }

                while(!_tokens.isEmpty() && _instance != null)
                {
                    long now = IceInternal.Time.currentMonotonicTimeMillis();
                    Token first = _tokens.first();
                    if(first.scheduledTime <= now)
                    {
                        _tokens.remove(first);
                        token = first;
                        if(token.delay == 0)
                        {
                            _tasks.remove(token.task);
                        }
                        break;
                    }

                    _wakeUpTime = first.scheduledTime;
                    while(true)
                    {
                        try
                        {
                            wait(first.scheduledTime - now);
                            break;
                        }
                        catch(java.lang.InterruptedException ex)
                        {
                        }
                    }
                }

                if(_instance == null)
                {
                    break;
                }
            }

            if(token != null)
            {
                try
                {
                    token.task.runTimerTask();
                }
                catch(Exception ex)
                {
                    synchronized(this)
                    {
                        if(_instance != null)
                        {
                            String s = "unexpected exception from task run method in timer thread:\n" + Ex.toString(ex);
                            _instance.initializationData().logger.error(s);
                        }
                    }
                }
            }
        }
    }

    static private class Token implements Comparable<Token>
    {
        public
        Token(long scheduledTime, int id, long delay, TimerTask task)
        {
            this.scheduledTime = scheduledTime;
            this.id = id;
            this.delay = delay;
            this.task = task;
        }

        public int
        compareTo(Token r)
        {
            //
            // Token are sorted by scheduled time and token id.
            //
            if(scheduledTime < r.scheduledTime)
            {
                return -1;
            }
            else if(scheduledTime > r.scheduledTime)
            {
                return 1;
            }

            if(id < r.id)
            {
                return -1;
            }
            else if(id > r.id)
            {
                return 1;
            }

            return 0;
        }

        public boolean
        equals(Object obj)
        {
            if(this == obj)
            {
                return true;
            }
            if(obj instanceof Token)
            {
                return compareTo((Token)obj) == 0;
            }
            return false;
        }

        public int
        hashCode()
        {
             return id ^ (int)scheduledTime;
        }

        long scheduledTime;
        int id; // Since we can't compare references, we need to use another id.
        long delay;
        TimerTask task;
    }

    private final java.util.SortedSet<Token> _tokens = new java.util.TreeSet<Token>();
    private final java.util.Map<TimerTask, Token> _tasks = new java.util.HashMap<TimerTask, Token>();
    private Instance _instance;
    private long _wakeUpTime = Long.MAX_VALUE;
    private int _tokenId = 0;
}
