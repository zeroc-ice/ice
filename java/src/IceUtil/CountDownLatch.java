// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceUtil;

//
// See java.util.CountDownLatch in Java 1.5
//

public class CountDownLatch
{
    public CountDownLatch(int count)
    {
        if(_count < 0)
        {
            throw new IllegalArgumentException("count < 0");
        }
        _count = count;
    }

    public synchronized void
    await() throws InterruptedException
    {
        while(_count > 0)
        {
            wait();
        }
    }
    
    public synchronized void
    countDown()
    {
        if(_count > 0 && --_count == 0)
        {
            notifyAll();
        }
    }
    
    public synchronized long
    getCount()
    {
        return _count;
    }

    private int _count;
}
