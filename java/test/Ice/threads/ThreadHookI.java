// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ThreadHookI implements Ice.ThreadNotification
{
    ThreadHookI()
    {
        _nextThreadId = 0;
    }

    public synchronized void
    start()
    {
        test(getThreadNumInternal() == -1);
        Thread t = Thread.currentThread();
        _threads.put(t, new Integer(_nextThreadId));
        _nextThreadId++;
    }

    public synchronized void
    stop()
    {
        Thread t = Thread.currentThread();
        test(_threads.containsKey(t));
        _threads.remove(t);
    }

    synchronized int
    getThreadNum()
    {
        return getThreadNumInternal();
    }

    synchronized int
    activeThreads()
    {
        return _threads.size();
    }

    private int
    getThreadNumInternal()
    {
        Thread t = Thread.currentThread();
        Integer i = (Integer)_threads.get(t);
        if(i == null)
        {
            return -1;
        }
        else
        {
            return i.intValue();
        }
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private java.util.Map _threads = new java.util.HashMap();
    private int _nextThreadId;
}
