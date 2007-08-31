// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

interface TimerTask
{
    void run();
};

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
            return;
        }

        final Token token = new Token(System.currentTimeMillis() + delay, ++_tokenId, 0, task);

        Object previous = _tasks.put(task, token);
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
            return;
        }

        final Token token = new Token(System.currentTimeMillis() + period, ++_tokenId, period, task);

        Object previous = _tasks.put(task, token);
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

        Token token = (Token)_tasks.remove(task);
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
        IceUtil.Assert.FinalizerAssert(_instance == null);
        
        super.finalize();
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
                            token.scheduledTime = System.currentTimeMillis() + token.delay;
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
                    long now = System.currentTimeMillis();
                    Token first = (Token)_tokens.first();
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
                    token.task.run();
                }
                catch(Exception ex)
                {
                    synchronized(this)
                    {
                        if(_instance != null)
                        {
                            java.io.StringWriter sw = new java.io.StringWriter();
                            java.io.PrintWriter pw = new java.io.PrintWriter(sw);
                            ex.printStackTrace(pw);
                            pw.flush();
                            String s = "unexpected exception from task run method in timer thread:\n" + sw.toString();
                            _instance.initializationData().logger.error(s);
                        }
                    }
                } 
            }
        }
    }

    static private class Token implements Comparable
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
        compareTo(Object o)
        {
            //
            // Token are sorted by scheduled time and token id.
            //
            Token r = (Token)o;
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

        long scheduledTime;
        int id; // Since we can't compare references, we need to use another id.
        long delay;
        TimerTask task;
    }

    private final java.util.SortedSet _tokens = new java.util.TreeSet();
    private final java.util.Map _tasks = new java.util.HashMap();
    private Instance _instance;
    private long _wakeUpTime = Long.MAX_VALUE;
    private int _tokenId = 0;
}
