// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
